/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "nano_helpers.h"
#include "submenus.h"
#include "nano_parse.h"
#include "nano_network.h"
#include "confirmation.h"

/*
 * Contacts fields:
 *     * name
 *     * address
 */

static const char TAG[] = "nano_contacts";
static const char TITLE[] = "Contacts";
static const char TITLE_SEND[] = "Send";

/* Static Function Declaration */
static lv_res_t processing_cb_1( lv_obj_t *dummy );
static void processing_cb_2( void *param );
static void processing_cb_3( nl_block_t *block, void *param, lv_obj_t *scr );
static void processing_cb_4( bool confirm, void *param );
static void processing_cb_4_1( void *param );
static void processing_cb_5( uint64_t work, void *param, lv_obj_t *scr );
static void processing_cb_6( esp_err_t status, void *param, lv_obj_t *scr);

typedef struct {
    struct{
        lv_obj_t *contacts;                /**< Contacts list */
        lv_obj_t *progress;                /**< Displays progress bar of processing transaction*/
        lv_obj_t *entry;                   /**< Screen to enter send amount */
        lv_obj_t *error;                   /**< DIsplay error */
    } scr;

    struct{
        nl_block_t *frontier; /**< Account frontier; an account cannot send funds unless it has a frontier */
        nl_block_t send;     /**< The block we want to sign in */
    } block;

    struct{
        cJSON *root;        /**< Root JSON config node */
        cJSON *contacts;    /**< Contacts array node */
        uint32_t account_index; /**< Local account derivation index */
    } cfg;

    mbedtls_mpi transaction_amount;     /**< bignum to hold precise transaction amount */

    int8_t contact_index;      /**< Selected contact index */

} send_obj_t;

static send_obj_t *d = NULL; /* Send Context */

/* CLEANUP/BACK CALLBACKS */

/**
 * @brief cleanup the entire send context.
 *
 * Called upon exit, completion, OOM-errors, or corrupt data.
 */
static void cleanup_complete( void *param ) {
    /* Screen cleanup */
    if(d->scr.contacts) jolt_gui_obj_del(d->scr.contacts);
    if(d->scr.progress) jolt_gui_obj_del(d->scr.progress);
    if(d->scr.entry)    jolt_gui_obj_del(d->scr.entry);
    if(d->scr.error)    jolt_gui_obj_del(d->scr.error);

    /* Block Cleanup */
    if(d->block.frontier) free(d->block.frontier);

    /* Config JSON cleanup */
    if(d->cfg.root) jolt_json_del(d->cfg.root);

    /* BigNum Cleanup */
    mbedtls_mpi_free(&d->transaction_amount);

    /* Context Deletion */
    free(d);
    d = NULL;
}

/**
 * @brief cleanup the entire send context.
 *
 * Called upon exit, completion, OOM-errors, or corrupt data.
 */
static lv_res_t cleanup_complete_cb( lv_obj_t *dummy ) {
    cleanup_complete(NULL);

    return LV_RES_INV;
}

static lv_res_t back_to_entry_cb( lv_obj_t *dummy ) {
    /* Screen cleanup */
    if(d->scr.progress) jolt_gui_obj_del(d->scr.progress);
    d->scr.progress = NULL;
    if(d->scr.error)    jolt_gui_obj_del(d->scr.error);
    d->scr.error = NULL;

    /* Set the first entry position to the one's place */
    jolt_gui_scr_digit_entry_set_pos(d->scr.entry, CONFIG_JOLT_NANO_SEND_DECIMALS);

    return LV_RES_INV;
}

static lv_res_t entry_back_cb(lv_obj_t *btn) {
    jolt_gui_obj_del(d->scr.entry);
    d->scr.entry = NULL;
    return LV_RES_INV;
}


/**
 * @brief Convert the user-entered NANO value into raw bignum.
 * @param[in] num_scr Number entry screen to get values from
 * @param[out] val BigInt to populate with raw value.
 */
static void entry_to_mpi(lv_obj_t *num_scr, mbedtls_mpi *val ) {
    /* Populate transaction Amount.
     *     1. Convert the user entered value into a string (no decimal, dealing with raw)
     *     2. Tack on enough 0's to agree with the decimal place in the GUI
     *     3. Convert the string into an mbedtls_mpi.
     */

    uint8_t user_entry[CONFIG_JOLT_NANO_SEND_DIGITS];
    char dest_amount_buf[40] = { 0 };
    uint8_t i;

    /* Get the user-entered array */
    jolt_gui_scr_digit_entry_get_arr(num_scr, user_entry, sizeof(user_entry));

    /* Copy over the user_entry array and convert to ascii */
    for(i = 0; i < CONFIG_JOLT_NANO_SEND_DIGITS; i++){
        dest_amount_buf[i] = user_entry[i] + '0';
    }

    /* Add enough zeros to represent the value in raw */
    for( uint8_t j = CONFIG_JOLT_NANO_SEND_DECIMALS; j < 30 ; i++, j++ ){
        dest_amount_buf[i] = '0';
    }

    mbedtls_mpi_read_string(val, 10, dest_amount_buf);
}

/**
 * @brief Prompts for PIN to get account access
 */
static lv_res_t processing_cb_1( lv_obj_t *num_scr ){
    /* populates dst_address */
    cJSON *contact, *json_address;

    contact = cJSON_GetArrayItem(d->cfg.contacts, d->contact_index);
    if( NULL == contact ) {
        ESP_LOGE(TAG, "Index out of range");
        goto exit;
    }

    {
        char *dst_address;
        json_address = cJSON_Get(contact, "address");
        if( NULL == json_address ) {
            ESP_LOGE(TAG, "Contact didn't have field \"address\"");
            goto exit;
        }
        dst_address = cJSON_GetStringValue( json_address );
        ESP_LOGD(TAG, "Contact address: %s", dst_address);
        nl_address_to_public(d->block.send.link, dst_address);
    }

    /* Populate transaction Amount. */
    entry_to_mpi(num_scr, &d->transaction_amount);

    /* Prompt for pin to derive public address */
    vault_refresh(NULL, processing_cb_2, NULL);

    return LV_RES_OK;

exit:
    cleanup_complete(NULL);
    return LV_RES_OK;
}


/**
 * @brief Creates loading screen, gets frontier
 */
static void processing_cb_2( void *param ) {
    char my_address[ADDRESS_BUF_LEN];

    d->scr.progress = jolt_gui_scr_loadingbar_create( TITLE_SEND );
    if( NULL == d->scr.progress) {
        ESP_LOGE(TAG, "Failed to allocate loadingbar screen");
        jolt_gui_scr_err_create(JOLT_GUI_ERR_OOM);
    }
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Getting Frontier", 10);

    /***********************************************
     * Get My Public Key, Private Key, and Address *
     ***********************************************/
    if( !nano_index_get_private_public_address(NULL, d->block.send.account, my_address, d->cfg.account_index) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "My Address: %s\n", my_address);

    /* Fetch frontier block (if not already fetched) */
    if( NULL == d->block.frontier ) {
        nano_network_frontier_block( my_address, processing_cb_3, NULL, d->scr.progress );
    }
    else {
        processing_cb_3( d->block.frontier, NULL, NULL );
    }

    return;

exit:
    cleanup_complete( NULL );
    return;
}

/**
 * @brief Craft send block
 */
static void processing_cb_3( nl_block_t *frontier_block, void *param, lv_obj_t *scr ) {

    if( NULL == frontier_block ) {
        /* No frontier blocks */
        ESP_LOGI(TAG, "No frontier block.");
        d->scr.error = jolt_gui_scr_text_create(TITLE, "Account not found");
        jolt_gui_scr_set_back_action(d->scr.error, cleanup_complete_cb);
        return;
    }

    d->block.frontier = frontier_block;

    /*****************
     * Check Balance *
     *****************/
    ESP_LOGD(TAG, "Verifying sufficient balance");
    if (mbedtls_mpi_cmp_mpi(&d->block.frontier->balance, &d->transaction_amount) == -1) {
        ESP_LOGW(TAG, "Insufficent Funds.");
        d->scr.error = jolt_gui_scr_text_create(TITLE, "Insufficient Funds");
        jolt_gui_scr_set_back_action(d->scr.error, back_to_entry_cb);
        return;
    }

    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Creating Block", 30);

    /*********************
     * Populate send block *
     *********************/
    nl_block_compute_hash(d->block.frontier, d->block.send.previous);
    memcpy(d->block.send.representative, d->block.frontier->representative, sizeof(uint256_t));
    mbedtls_mpi_sub_abs(&d->block.send.balance, &d->block.frontier->balance, &d->transaction_amount);

    /******************************
     * Create confirmation screen *
     ******************************/
    ESP_LOGD(TAG, "Requesting user to confirm transaction");
    nano_confirm_block(d->block.frontier, &d->block.send, processing_cb_4, NULL);
}

/**
 * @brief Refresh vault for signing
 */
static void processing_cb_4( bool confirm, void *param ) {
    if( !confirm ) {
        cleanup_complete(NULL);
        return;
    }

    /* Refresh vault to get private key */
    vault_refresh(cleanup_complete, processing_cb_4_1, NULL);

    return;
}

static void processing_cb_4_1( void *param ){
    /* Generate Signature */

    /* Sign Send Block */
    ESP_LOGI(TAG, "Signing Block");
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Signing", 70);
    {
        CONFIDENTIAL uint256_t private_key;
        jolt_err_t res;

        if( !nano_index_get_private(private_key, d->cfg.account_index) ) {
            ESP_LOGI(TAG, "Error getting private key");
            goto exit;
        }
        res = nl_block_sign(&d->block.send, private_key);
        sodium_memzero(private_key, sizeof(private_key));
        if(E_SUCCESS != res ) {
            ESP_LOGI(TAG, "Error Signing Block");
            goto exit;
        }
    }

    /* Get PoW */
    ESP_LOGD(TAG, "Fetching PoW");
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Fetching PoW", 60);
    nano_network_work_bin( d->block.send.previous, processing_cb_5, d, d->scr.progress );

    return;
exit:
    cleanup_complete( NULL);
    return;
}

/**
 * @brief Broadcast
 */
static void processing_cb_5( uint64_t work, void *param, lv_obj_t *scr ){
    send_obj_t *d = param;

    if( 0 == work ) {
        d->scr.error = jolt_gui_scr_text_create(TITLE, "Failed to fetch work");
        jolt_gui_scr_set_action(d->scr.error, cleanup_complete_cb);
        return;
    }
    d->block.send.work = work;

    /* Broadcast signed block */
    ESP_LOGI(TAG, "Broadcasting Block");
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Broadcasting", 100);
    nano_network_process( &d->block.send, processing_cb_6, NULL, NULL); // Cannot reliably cancel at this stage.
    return;
}

static void processing_cb_6( esp_err_t status, void *param, lv_obj_t *scr){
    if( ESP_OK == status ) {
        jolt_gui_scr_text_create(TITLE_SEND, "Transaction Complete");
    }
	else {
        jolt_gui_scr_text_create(TITLE_SEND, "Transaction Failed");
    }
    cleanup_complete(NULL);
    return;
}

/**
 * @brief Create the amount-entry screen
 */
static lv_res_t contact_cb( lv_obj_t *btn_sel ) {
    d->contact_index = lv_list_get_btn_index(NULL, btn_sel);

    ESP_LOGD(TAG, "Creating Digit Entry Screen");
    d->scr.entry = jolt_gui_scr_digit_entry_create(TITLE_SEND,
        CONFIG_JOLT_NANO_SEND_DIGITS, CONFIG_JOLT_NANO_SEND_DECIMALS);
    if( NULL == d->scr.entry ) {
        ESP_LOGE(TAG, "Failed to create digit entry screen");
    }
    /* Set the first entry position to the one's place */
    jolt_gui_scr_digit_entry_set_pos(d->scr.entry, CONFIG_JOLT_NANO_SEND_DECIMALS);

    jolt_gui_scr_set_enter_action(d->scr.entry, processing_cb_1);
    jolt_gui_scr_set_back_action(d->scr.entry, entry_back_cb);

    return LV_RES_OK;
}

lv_res_t menu_nano_contacts( lv_obj_t *btn ) {
    send_obj_t *d = NULL;

    /* Create context */
    ESP_LOGD(TAG, "Allocating space for send context");
    d = malloc(sizeof(send_obj_t));
    if( NULL == d ){
        ESP_LOGE(TAG, "Could not allocate memory for send_obj_t");
        return LV_RES_OK;
    }
    memset(d, 0, sizeof(send_obj_t));
    nl_block_init(&d->block.send);
    d->block.send.type = STATE;
    mbedtls_mpi_init(&d->transaction_amount);
    d->contact_index = -1;

    /* Read Config */
    d->cfg.root = nano_get_json(); // never fails
    d->cfg.contacts = cJSON_Get(d->cfg.root, "contacts");

    d->cfg.account_index = nano_index_get( d->cfg.root );

    if( NULL == d->cfg.contacts || 0 == cJSON_GetArraySize(d->cfg.contacts) ) {
        d->scr.error = jolt_gui_scr_text_create(TITLE, "No Contacts");
        jolt_gui_scr_set_back_action(d->scr.error, cleanup_complete_cb);
        return LV_RES_OK;
    }

    ESP_LOGD(TAG, "Creating contacts menu");
    d->scr.contacts = jolt_gui_scr_menu_create(TITLE);
    if( NULL == d->scr.contacts ){
        ESP_LOGE(TAG, "Failed to create contacts menu");
        cleanup_complete(NULL);
        return LV_RES_OK;
    }
    jolt_gui_scr_set_back_action(d->scr.contacts, cleanup_complete_cb);
    jolt_gui_scr_set_param(d->scr.contacts, d);

    ESP_LOGD(TAG, "Iterating through saved contacts");
    cJSON *contact = NULL;
    cJSON_ArrayForEach(contact, d->cfg.contacts){
        cJSON *elem;
        elem = cJSON_Get(contact, "name");
        ESP_LOGD(TAG, "Adding contact list element-name \"%s\"",
                cJSON_GetStringValue(elem));
        BREAK_IF_NULL(jolt_gui_scr_menu_add(d->scr.contacts,
                    NULL, cJSON_GetStringValue(elem), contact_cb));
    }

    return LV_RES_OK;
}

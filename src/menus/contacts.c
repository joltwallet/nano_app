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

static cJSON *json = NULL;
static cJSON *contacts = NULL;
static int32_t idx;

lv_obj_t *scr_contacts     = NULL; /**< Contacts list */
lv_obj_t *scr_digit_entry  = NULL; /**< Screen to enter send amount */
lv_obj_t *scr_confirmation = NULL; /**< Yes/No confirmation screen */
lv_obj_t *scr_progress     = NULL; /**< Displays progress bar of processing transaction*/

/* Static Function Declaration */
static lv_res_t processing_cb_1( lv_obj_t *dummy );
static void processing_cb_2( void *param );
static void processing_cb_3( nl_block_t *block, void *param, lv_obj_t *scr );
static void processing_cb_4( bool confirm, void *param );
static void processing_cb_5( uint64_t work, void *param, lv_obj_t *scr );
static void processing_cb_6( esp_err_t status, void *param, lv_obj_t *scr);

typedef struct {
    lv_obj_t *scr;             /**< loading/progress screen */
    CONFIDENTIAL uint256_t my_private_key;
    uint256_t my_public_key;
    char my_address[ADDRESS_BUF_LEN];
    nl_block_t frontier_block;
    nl_block_t send_block;     /**< The block we want to sign in */
    mbedtls_mpi transaction_amount;
} send_obj_t;

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
 * @brief De-allocate all the stuff 
 */
static lv_res_t cleanup_cb( lv_obj_t *obj ) {
    jolt_gui_obj_del(scr_contacts);
    jolt_json_del(json);
    json = NULL;
    contacts = NULL;
    idx = 0;
    return LV_RES_INV;
}

/**
 * @brief Prompts for PIN to get account access
 */
static lv_res_t processing_cb_1( lv_obj_t *num_scr ){
    /* Parse JSON data for this contact */
    send_obj_t *d;

    d = malloc(sizeof(send_obj_t));
    if( NULL == d ){
        ESP_LOGE(TAG, "Could not allocate memory for send_obj_t");
        goto exit;
    }
    memset(d, 0, sizeof(send_obj_t));
    nl_block_init(&d->frontier_block);
    nl_block_init(&d->send_block);
    d->send_block.type = STATE;
    mbedtls_mpi_init(&d->transaction_amount);

    /* populates dst_address */
    cJSON *contact, *json_address;

    contact = cJSON_GetArrayItem(contacts, idx);
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
        nl_address_to_public(&d->send_block.link, dst_address);
    }

    /* Populate transaction Amount. */
    entry_to_mpi(num_scr, &d->transaction_amount);

    /* Prompt for pin */
    vault_refresh(NULL, processing_cb_2, d); //todo failure cleanup

    return LV_RES_OK;

exit:
    return LV_RES_OK;
}


/**
 * @brief Creates loading screen, gets frontier
 */
static void processing_cb_2( void *param ) {
    send_obj_t *d = param;

    scr_progress = jolt_gui_scr_loadingbar_create( TITLE_SEND );
    if( NULL == scr_progress) {
        ESP_LOGE(TAG, "Failed to allocate loadingbar screen");
        jolt_gui_scr_err_create(JOLT_GUI_ERR_OOM);
    }
    d->scr = scr_progress;
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Getting Frontier", 10);

    /***********************************************
     * Get My Public Key, Private Key, and Address *
     ***********************************************/
    if( !nano_get_private_public_address(d->my_private_key, d->send_block.account, d->my_address) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "My Address: %s\n", d->my_address);

    nano_network_frontier_block( d->my_address, processing_cb_3, d, d->scr );

    return;
exit:
#if 0
    in_progress = false;
    if( NULL != scr ) {
        jolt_gui_obj_del(scr);
    }
    if( NULL != d ) {
        free(d);
    }
#endif
    return;
}

/**
 * @brief Craft send block
 */
static void processing_cb_3( nl_block_t *frontier_block, void *param, lv_obj_t *scr ) {
    send_obj_t *d = param;

    if( NULL == frontier_block ) {
        /* No frontier blocks */
        ESP_LOGI(TAG, "No frontier block.");
        jolt_gui_scr_text_create(TITLE, "Account not found");
        jolt_gui_obj_del(d->scr);
        goto exit;
    }
    memcpy(&d->frontier_block, frontier_block, sizeof(nl_block_t));
    free(frontier_block);

    /*****************
     * Check Balance *
     *****************/
    if (mbedtls_mpi_cmp_mpi(&d->frontier_block.balance, &d->transaction_amount) == -1) {
        ESP_LOGI(TAG, "Insufficent Funds.");
        goto exit;
    }

    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Creating Block", 30);

    /*********************
     * Create send block *
     *********************/
    nl_block_compute_hash(&d->frontier_block, d->send_block.previous);
    memcpy(d->send_block.representative, d->frontier_block.representative, sizeof(uint256_t));
    mbedtls_mpi_sub_abs(&d->send_block.balance, &d->frontier_block.balance, &d->transaction_amount);

    /******************************
     * Create confirmation screen *
     ******************************/
    nano_confirm_block(&d->frontier_block, &d->send_block, processing_cb_4, d);

exit:
    return;
}

/**
 * @brief Get PoW
 */
static void processing_cb_4( bool confirm, void *param ) {
    send_obj_t *d = param;

    if( !confirm ) {
        //todo: cleanup
        goto exit;
    }

    /* Get PoW */
    ESP_LOGD(TAG, "Fetching PoW");
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Fetching PoW", 60);
    nano_network_work( &d->send_block.previous, processing_cb_5, d, &d->scr );

    return;
exit:
    return;
}

/**
 * @brief Signed and Broadcast if confirmed
 */
static void processing_cb_5( uint64_t work, void *param, lv_obj_t *scr ){
    send_obj_t *d = param;

    if( 0 == work ) {
        goto exit;
    }
    /* Sign Send Block */
    ESP_LOGI(TAG, "Signing Block");
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Signing", 70);
	if( E_SUCCESS != nl_block_sign(&d->send_block, d->my_private_key) ) {
		ESP_LOGI(TAG, "Error Signing Block");
		goto exit;
	}

    /* Broadcast signed block */
    ESP_LOGI(TAG, "Broadcasting Block");
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Broadcasting", 100);
    nano_network_process( &d->send_block, processing_cb_6, d, NULL); // Cannot reliably cancel at this stage.
    return;
exit:
    return;
}

static void processing_cb_6( esp_err_t status, void *param, lv_obj_t *scr){
    send_obj_t *d = param;
    if( ESP_OK == status ) {
        jolt_gui_scr_text_create(TITLE_SEND, "Transaction Complete");
    }
	else {
        jolt_gui_scr_text_create(TITLE_SEND, "Transaction Failed");
    }
    // todo: cleanup
    return;
}

#if 0
/**
 * @brief Create the confirmation screen.
 */
static lv_res_t number_cb( lv_obj_t *num_scr ) {
    // todo get rid of this first confirmation
    char buf[128];
    cJSON *contact, *json_name, *json_address;
    float disp_amount;

    disp_amount = (float) jolt_gui_scr_digit_entry_get_double(num_scr);
    ESP_LOGD(TAG, "Send amount: %f NANO", disp_amount);

    contact = cJSON_GetArrayItem(contacts, idx);
    if( NULL == contact ) {
        ESP_LOGE(TAG, "Index out of range");
        goto exit;
    }

    json_name = cJSON_Get(contact, "name");
    if( NULL == json_name ) {
        ESP_LOGE(TAG, "Contact didn't have field \"name\"");
        goto exit;
    }
    ESP_LOGD(TAG, "Contact name: %s", cJSON_GetStringValue(json_name));

    json_address = cJSON_Get(contact, "address");
    if( NULL == json_address ) {
        ESP_LOGE(TAG, "Contact didn't have field \"address\"");
        goto exit;
    }
    dst_address = cJSON_GetStringValue( json_address );
    ESP_LOGD(TAG, "Contact address: %s", dst_address);

    snprintf(buf, sizeof(buf), "Would you like to send %d NANO to %s\n", disp_amount, dst_address );
    scr_confirmation = jolt_gui_scr_text_create(TITLE_SEND, buf);
    jolt_gui_scr_scroll_add_monospace_text( scr_confirmation, dst_address );
    jolt_gui_scr_set_enter_action(scr_confirmation, processing_cb_1);

    return LV_RES_OK;

exit:
    return LV_RES_OK;
}
#endif

/**
 * @brief Create the amount-entry screen
 */
static lv_res_t contact_cb( lv_obj_t *btn_sel ) {
    lv_obj_t *scr = NULL;
    idx = lv_list_get_btn_index(NULL, btn_sel);

    scr = jolt_gui_scr_digit_entry_create(TITLE_SEND,
        CONFIG_JOLT_NANO_SEND_DIGITS, CONFIG_JOLT_NANO_SEND_DECIMALS);
    jolt_gui_scr_set_enter_action(scr, processing_cb_1);

    return LV_RES_OK;
}

lv_res_t menu_nano_contacts( lv_obj_t *btn ) {
    /* Read Config */
    cJSON *contact = NULL;

    json = nano_get_json();
    contacts = cJSON_Get(json, "contacts");

    int n_contacts = cJSON_GetArraySize(contacts);

    if( 0 == n_contacts ) {
        jolt_gui_scr_text_create(TITLE, "No Contacts");
        goto exit;
    }

    ESP_LOGD(TAG, "Creating contacts menu");
    scr_contacts = jolt_gui_scr_menu_create(TITLE);
    jolt_gui_scr_set_back_action(scr_contacts, cleanup_cb);

    ESP_LOGD(TAG, "Iterating through saved contacts");
    cJSON_ArrayForEach(contact, contacts){
        cJSON *elem;
        elem = cJSON_Get(contact, "name");
        ESP_LOGD(TAG, "Adding contact list element-name \"%s\"", cJSON_GetStringValue(elem));
        jolt_gui_scr_menu_add(scr_contacts, NULL, cJSON_GetStringValue(elem), contact_cb);
    }
    return LV_RES_OK;

exit:
    jolt_json_del(json);
    return LV_RES_OK;
}

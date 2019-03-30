/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "nano_parse.h"

#include "../confirmation.h"
#include "../nano_helpers.h"
#include "../nano_network.h"
#include "submenus.h"
#include "sdkconfig.h"

static const char TAG[] = "nano_receive";
static const char TITLE[] = "Receive Nano";

typedef struct {
    struct{
        lv_obj_t *progress;   /** Progress bar screen */
    } scr;

    struct{
        nl_block_t *frontier; /**< Frontier of our account chain */
        nl_block_t receive;   /**< The block we want to sign in */
        uint8_t open:1;       /**< indicates if Receive Block needs to be an "open" state type */
    } block;

    char my_address[ADDRESS_BUF_LEN]; /**< Address in xrb_ form */
    mbedtls_mpi transaction_amount;      /**< Incoming transaction amount in raw */
} receive_obj_t;

static void step1( void *param );
static void step2(hex256_t pending_block_hash, mbedtls_mpi *amount, void *param, lv_obj_t *scr);
static void step3( nl_block_t *frontier_block, void *param, lv_obj_t *scr );
static void step4( bool confirm, void *param);
static void step4_1( void *param );
static void step5( uint64_t work, void *param, lv_obj_t *scr);
static void step6( esp_err_t status, void *param, lv_obj_t *scr);

lv_res_t menu_nano_receive( lv_obj_t *btn ) {
    vault_refresh(NULL, step1, NULL);
    return LV_RES_OK;
}

/**
 * @brief cleanup the entire send context.
 *
 * Called upon exit, completion, OOM-errors, or corrupt data.
 */
static void cleanup_complete( void *param ) {
    receive_obj_t *d = param;

    /* Screen cleanup */
    if(d->scr.progress) jolt_gui_obj_del(d->scr.progress);

    /* Block Cleanup */
    if(d->block.frontier) free(d->block.frontier);

    /* BigNum Cleanup */
    mbedtls_mpi_free(&d->transaction_amount);

    /* Context Deletion */
    free(d);
    d = NULL;
}

static void step1( void *param ) {
    receive_obj_t *d = NULL;

    d = malloc( sizeof(receive_obj_t) );
    if( NULL == d ){
        ESP_LOGE(TAG, "Could not allocate memory for receive_obj_t");
        goto exit;
    }
    memset(d, 0, sizeof(receive_obj_t));
    nl_block_init(&d->block.receive);
    d->block.receive.type = STATE;
    mbedtls_mpi_init(&d->transaction_amount);

    /* Create Loading Screen */
    d->scr.progress = jolt_gui_scr_loadingbar_create(TITLE);
    if( NULL == d->scr.progress) {
        ESP_LOGE(TAG, "Failed to allocate loadingbar screen");
        jolt_gui_scr_err_create(JOLT_GUI_ERR_OOM);
        goto exit;
    }
    jolt_gui_scr_loadingbar_update( d->scr.progress, NULL, "", 0);

    /***********************************************
     * Get My Public Key, Private Key, and Address *
     ***********************************************/
    if( !nano_get_private_public_address(NULL, d->block.receive.account, d->my_address) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "My Address: %s\n", d->my_address);

    /*********************
     * Get Pending Block *
     *********************/
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Checking Pending", 10);
    nano_network_pending_hash( d->my_address, step2, d, d->scr.progress );

    return;

exit:
    cleanup_complete( d );
    return;
}

static void step2(hex256_t pending_block_hash, mbedtls_mpi *amount, void *param, lv_obj_t *scr) {
    receive_obj_t *d = param;

    /* Copy over the pending block hash */
    if( NULL == pending_block_hash || NULL == amount ) {
        /* No pending blocks */
        ESP_LOGI(TAG, "No pending blocks. %p %p", pending_block_hash, amount);
        jolt_gui_scr_text_create(TITLE, "No pending blocks found");
        goto exit;
    }
    ESP_LOGD(TAG, "Pending Hash: %s", pending_block_hash);
    sodium_hex2bin(d->block.receive.link, sizeof(uint256_t),
            pending_block_hash, sizeof(hex256_t), NULL, NULL, NULL);
    free(pending_block_hash);

    /* Copy over the receive amount */
    mbedtls_mpi_copy(&d->transaction_amount, amount);
    mbedtls_mpi_free(amount); 
    free(amount);
    amount = NULL;

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        /* Print the receive amount */
        char amount[66];
        size_t olen;
        if(mbedtls_mpi_write_string(&d->transaction_amount, 10, amount, sizeof(amount), &olen)){
            ESP_LOGE(TAG, "Unable to write string from mbedtls_mpi; olen: %d", olen);
        }
        ESP_LOGD(TAG, "Pending Amount: %s", amount);
    }
    #endif

    /***********************************
     * Get My Account's Frontier Block *
     ***********************************/
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Checking Account", 20);
    nano_network_frontier_block( d->my_address, step3, d, d->scr.progress );
    return;

exit:
    cleanup_complete( d );
    return;
}

static void step3( nl_block_t *frontier_block, void *param, lv_obj_t *scr ){
    receive_obj_t *d = param;

    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Creating Block", 30);

    if( NULL == frontier_block) {
        ESP_LOGI(TAG, "No frontier block found. Assuming Open");
        d->block.open = true;
        nl_address_to_public(d->block.receive.representative, 
                CONFIG_JOLT_NANO_DEFAULT_REPRESENTATIVE);
    }
    else {
        d->block.frontier = frontier_block;
        memcpy(d->block.receive.representative, d->block.frontier->representative, sizeof(uint256_t));
        nl_block_compute_hash(d->block.frontier, d->block.receive.previous);
    }

    mbedtls_mpi_add_abs(&(d->block.receive.balance), &d->transaction_amount,
            &d->block.frontier->balance);

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        char amount[66];
        size_t olen;
        mbedtls_mpi_write_string(&d->block.frontier->balance, 10, amount, sizeof(amount), &olen);
        ESP_LOGD(TAG, "Frontier Amount: %s", amount);
        mbedtls_mpi_write_string(&(d->block.receive.balance), 10, amount, sizeof(amount), &olen);
        ESP_LOGD(TAG, "New Block Amount: %s", amount);
        mbedtls_mpi_write_string(&d->transaction_amount, 10, amount, sizeof(amount), &olen);
        ESP_LOGD(TAG, "Transaction Amount: %s", amount);
    }
    #endif

    /*****************
     * Confirm Block *
     *****************/
    ESP_LOGI(TAG, "Signing Block");
    nano_confirm_block(d->block.frontier, &d->block.receive, step4, d);

    return;
}

static void step4( bool confirm, void *param) {
    receive_obj_t *d = param;

    if(!confirm) {
        goto exit;
    }

    /*****************
     * Refresh vault *
     *****************/
    vault_refresh(cleanup_complete, step4_1, d);

    return;

exit:
    cleanup_complete( d );
    return;
}

static void step4_1( void *param ) {
    receive_obj_t *d = param;

    /**************
     * Sign Block *
     **************/
    {
        CONFIDENTIAL uint256_t private_key;
        jolt_err_t res;

        if( !nano_get_private(private_key) ) {
            ESP_LOGI(TAG, "Error getting private key");
            goto exit;
        }
        res = nl_block_sign(&d->block.receive, private_key);
        sodium_memzero(private_key, sizeof(private_key));
        if(E_SUCCESS != res ) {
            ESP_LOGI(TAG, "Error Signing Block");
            goto exit;
        }
    }

    /**************
     * Fetch Work *
     **************/
    ESP_LOGI(TAG, "Fetching Work");
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Fetching Work", 50);
    if( d->block.open ){
        nano_network_work_bin( d->block.receive.account, step5, d, d->scr.progress );
    }
    else{
        nano_network_work_bin( d->block.receive.previous, step5, d, d->scr.progress );
    }

    return;

exit:
    cleanup_complete( d );
    return;
}


static void step5( uint64_t work, void *param, lv_obj_t *scr ) {
    receive_obj_t *d = param;

    if( 0 == work ) {
        goto exit;
    }
    d->block.receive.work = work;

    /*************
     * Broadcast *
     *************/
    jolt_gui_scr_loadingbar_update(d->scr.progress, NULL, "Broadcasting", 70);
    /* Not directly passing the progress screen because you cannot reliably cancel the
     * broadcasting step */
    nano_network_process( &d->block.receive, step6, d, NULL );

    return;

exit:
    cleanup_complete( d );
    return;
}

static void step6( esp_err_t status, void *param, lv_obj_t *scr) {
    receive_obj_t *d = param;

    cleanup_complete( d );

    if(ESP_OK != status) {
        goto exit;
    }

    jolt_gui_scr_text_create(TITLE, "Block Processed");
    return;

exit:
    jolt_gui_scr_text_create(TITLE, "Error Broadcasting");
    return;
}

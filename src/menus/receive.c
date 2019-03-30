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

static bool in_progress = false; /**< only allow a single receive operation at a time */

typedef struct {
    lv_obj_t *scr;
    hex256_t pending_hash;
    hex256_t frontier_hash;
    CONFIDENTIAL uint256_t my_private_key;
    uint256_t my_public_key;
    char my_address[ADDRESS_BUF_LEN];
    uint64_t proof_of_work;
    mbedtls_mpi transaction_amount;
    nl_block_t frontier_block; /**< Frontier of our account chain */
    nl_block_t receive_block;  /**< The block we want to sign in */
    bool open; // indicates if Receive Block needs to be an "open" state type
} receive_obj_t;

static void step1( void *param );
static void step2(hex256_t pending_block_hash, mbedtls_mpi *amount, void *param, lv_obj_t *scr);
static void step3( nl_block_t *frontier_block, void *param, lv_obj_t *scr );
static void step4( bool confirm, void *param);
static void step5( uint64_t work, void *param, lv_obj_t *scr);
static void step6( esp_err_t status, void *param, lv_obj_t *scr);

lv_res_t menu_nano_receive( lv_obj_t *btn ) {
    printf("refreshing vault %d\n", in_progress);
    vault_refresh(NULL, step1, NULL);
    return LV_RES_OK;
}

static void step1( void *param ) {
    lv_obj_t *scr = NULL;
    receive_obj_t *d = NULL;

    if( in_progress ) {
        ESP_LOGW(TAG, "Operation already in progress");
        return;
    }
    else {
        in_progress = true;
    }

    d = malloc( sizeof(receive_obj_t) );
    if( NULL == d ){
        ESP_LOGE(TAG, "Could not allocate memory for receive_obj_t");
        goto exit;
    }
    memset(d, 0, sizeof(receive_obj_t));
    nl_block_init(&d->frontier_block);
    nl_block_init(&d->receive_block);
    mbedtls_mpi_init(&d->transaction_amount);

    /* Create Loading Screen */
    scr = jolt_gui_scr_loadingbar_create(TITLE);
    d->scr = scr;
    if( NULL == scr) {
        ESP_LOGE(TAG, "Failed to allocate loadingbar screen");
        jolt_gui_scr_err_create(JOLT_GUI_ERR_OOM);
        goto exit;
    }
    jolt_gui_scr_loadingbar_update( scr, NULL, "", 0);

    /***********************************************
     * Get My Public Key, Private Key, and Address *
     ***********************************************/
    if( !nano_get_private_public_address(d->my_private_key, d->my_public_key, d->my_address) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "My Address: %s\n", d->my_address);

    /*********************
     * Get Pending Block *
     *********************/
    jolt_gui_scr_loadingbar_update(scr, NULL, "Checking Pending", 10);
    nano_network_pending_hash( d->my_address, step2, d, scr );

    return;

exit:
    in_progress = false;
    if( NULL != scr ) {
        jolt_gui_obj_del(scr);
    }
    if( NULL != d ) {
        free(d);
    }
    return;
}

static void step2(hex256_t pending_block_hash, mbedtls_mpi *amount, void *param, lv_obj_t *scr) {
    receive_obj_t *d = param;

    if( NULL == pending_block_hash || NULL == amount ) {
        /* No pending blocks */
        ESP_LOGI(TAG, "No pending blocks. %p %p", pending_block_hash, amount);
        jolt_gui_scr_text_create(TITLE, "No pending blocks found");
        goto exit;
    }

    memcpy(d->pending_hash, pending_block_hash, sizeof(hex256_t));
    free(pending_block_hash);
    pending_block_hash = NULL;
    ESP_LOGI(TAG, "Pending Hash: %s", d->pending_hash);

    memcpy(&d->transaction_amount, amount, sizeof(mbedtls_mpi));
    free(amount);
    amount = NULL;

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        char amount[66];
        size_t olen;
        if(mbedtls_mpi_write_string(&d->transaction_amount, 10, amount, sizeof(amount), &olen)){
            ESP_LOGE(TAG, "Unable to write string from mbedtls_mpi; olen: %d", olen);
        }
        ESP_LOGI(TAG, "Pending Amount: %s", amount);
    }
    #endif

    /***********************************
     * Get My Account's Frontier Block *
     ***********************************/
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Checking Account", 20);
    nano_network_frontier_block( d->my_address, step3, d, d->scr );
    return;

exit:
    if( NULL != pending_block_hash ) {
        free(pending_block_hash);
    }
    if( NULL!= amount ) {
        free(amount);
    }
    jolt_gui_obj_del(d->scr);
    free(d);
    in_progress = false;
    return;
}

static void step3( nl_block_t *frontier_block, void *param, lv_obj_t *scr ){
    receive_obj_t *d = param;

    if( NULL == frontier_block) {
        ESP_LOGI(TAG, "No frontier block found. Assuming Open");
        d->open = true;
    }
    else {
        memcpy(&d->frontier_block, frontier_block, sizeof(nl_block_t));
        free(frontier_block);
    }
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Creating Block", 30);

    /*****************************
     * Create receive/open block *
     *****************************/
    d->receive_block.type = STATE;
    // Frontier Hash is all zero if its an open
    nl_block_compute_hash(&d->frontier_block, d->receive_block.previous);
#if 0
    sodium_hex2bin(d->receive_block.previous, sizeof(uint256_t),
            d->frontier_hash, sizeof(hex256_t), NULL, NULL, NULL);
#endif
    memcpy(d->receive_block.account, d->my_public_key, sizeof(d->my_public_key));
    nl_address_to_public(d->receive_block.representative, 
            CONFIG_JOLT_NANO_DEFAULT_REPRESENTATIVE);
    sodium_hex2bin(d->receive_block.link, sizeof(d->receive_block.link),
            d->pending_hash, sizeof(d->pending_hash), NULL, NULL, NULL);
    mbedtls_mpi_add_abs(&(d->receive_block.balance), &d->transaction_amount,
            &d->frontier_block.balance);

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        char amount[66];
        size_t olen;
        mbedtls_mpi_write_string(&d->frontier_block.balance, 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "Frontier Amount: %s", amount);
        mbedtls_mpi_write_string(&(d->receive_block.balance), 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "New Block Amount: %s", amount);
        mbedtls_mpi_write_string(&d->transaction_amount, 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "Transaction Amount: %s", amount);
    }
    #endif

    /**************************
     * Confirm Block *
     **************************/
    ESP_LOGI(TAG, "Signing Block");
    nano_confirm_block(&d->frontier_block, &d->receive_block, step4, d);

    return;
}

static void step4( bool confirm, void *param) {
    receive_obj_t *d = param;

    if(!confirm) {
        goto exit;
    }

    /**************
     * Sign Block *
     **************/

    if( E_SUCCESS != nl_block_sign(&d->receive_block, d->my_private_key) ) {
        ESP_LOGI(TAG, "Error Signing Block");
        goto exit;
    }

    /**************
     * Fetch Work *
     **************/
    ESP_LOGI(TAG, "Fetching Work");
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Fetching Work", 50);
    if( d->open ){
        hex256_t work_hex;
        sodium_bin2hex(work_hex, sizeof(work_hex),
                d->my_public_key, sizeof(d->my_public_key));
        nano_network_work( work_hex, step5, d, d->scr );
    }
    else{
        nano_network_work_bin( d->receive_block.previous, step5, d, d->scr );
    }

    return;

exit:
    jolt_gui_obj_del(d->scr);
    free(d);
    in_progress = false;
    return;
}

static void step5( uint64_t work, void *param, lv_obj_t *scr ) {
    receive_obj_t *d = param;

    if( 0 == work ) {
        goto exit;
    }
    d->receive_block.work = work;

    /*************
     * Broadcast *
     *************/
    jolt_gui_scr_loadingbar_update(d->scr, NULL, "Broadcasting", 70);
    /* Not directly passing the screen because you cannot reliably cancel the
     * broadcasting step */
    nano_network_process( &d->receive_block, step6, d, NULL );

    return;

exit:
    jolt_gui_obj_del(d->scr);
    free(d);
    in_progress = false;
    return;
}

static void step6( esp_err_t status, void *param, lv_obj_t *scr) {
    receive_obj_t *d = param;
    jolt_gui_scr_del(d->scr);

    if(ESP_OK != status) {
        goto exit;
    }

    jolt_gui_scr_text_create(TITLE, "Block Processed");

    return;

exit:
    jolt_gui_scr_text_create(TITLE, "Error Broadcasting");
    in_progress = false;
    return;
}

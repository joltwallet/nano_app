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
#include "submenus.h"
#include "sdkconfig.h"


static const char TAG[] = "nano_receive";
static const char TITLE[] = "Receive Nano";

void nano_receive_task(void *);

lv_action_t menu_nano_receive( lv_obj_t *btn ) {
    xTaskCreate(nano_receive_task,
            "nano_receive", 16000, //todo: optimize this number
            NULL, 10, NULL);
    return LV_RES_OK;
}

void nano_receive_task(void *param) {
    /*
     * Blocks involved:
     * pending_block - The send we want to sign in
     * receive_block - block we are creating
     * frontier_block - frontier of our account chain
     */
    
    hex256_t pending_hash = { 0 };
    hex256_t frontier_hash = { 0 };
    CONFIDENTIAL uint256_t my_private_key;
    uint256_t my_public_key;
    char my_address[ADDRESS_BUF_LEN];
    uint64_t proof_of_work;
    mbedtls_mpi transaction_amount;

    nl_block_t frontier_block;
    nl_block_init(&frontier_block);

    jolt_gui_sem_take();
    lv_obj_t *scr_loading = jolt_gui_scr_loading_create("Receive");
    // todo: set a back action that can abort the network task.
    jolt_gui_sem_give();

    /***********************************************
     * Get My Public Key, Private Key, and Address *
     ***********************************************/
    if( !nano_get_private_public_address(my_private_key, my_public_key, my_address) ) {
        goto exit;
    }
    ESP_LOGI(TAG, "My Address: %s\n", my_address);

    /*********************
     * Get Pending Block *
     *********************/
    // Outcome: 
    //     * pending_hash, pending_amount
    // Returns if no pending blocks. Pending_amount doesn't need to be verified
    // since nothing malicious can be done with a wrong pending_amount.
    jolt_gui_sem_take();
    jolt_gui_scr_loading_update(scr_loading, NULL, "Checking Pending", 10);
    jolt_gui_sem_give();
    
    /* Search for pending block(s) */
    mbedtls_mpi_init(&transaction_amount);
    switch( nanoparse_web_pending_hash(my_address, pending_hash, &transaction_amount) ) {
        case E_SUCCESS:
            break;
        default:
            jolt_gui_sem_take();
            jolt_gui_scr_text_create(TITLE, "No pending blocks found.");
            jolt_gui_sem_give();
            goto exit;
    }

    ESP_LOGI(TAG, "Pending Hash: %s", pending_hash);

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        char amount[66];
        size_t olen;
        if(mbedtls_mpi_write_string(&transaction_amount, 10, amount, sizeof(amount), &olen)){
            ESP_LOGE(TAG, "Unable to write string from mbedtls_mpi; olen: %d", olen);
        }
        ESP_LOGI(TAG, "Pending Amount: %s", amount);
    }
    #endif

    /***********************************
     * Get My Account's Frontier Block *
     ***********************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    bool open; // Receive Block needs to be an "open" state type
    memcpy(frontier_block.account, my_public_key, sizeof(my_public_key));

    jolt_gui_sem_take();
    jolt_gui_scr_loading_update(scr_loading, NULL, "Checking Account", 20);
    jolt_gui_sem_give();

    ESP_LOGI(TAG, "Fetching Frontier Block Contents");
    switch( nanoparse_web_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            ESP_LOGI(TAG, "Creating RECEIVE State Block");
            open = false;

            ESP_LOGI(TAG, "Locally recomputing Frontier Hash");
            uint256_t frontier_hash_bin;
            nl_block_compute_hash(&frontier_block, frontier_hash_bin);
            sodium_bin2hex(frontier_hash, sizeof(frontier_hash),
                    frontier_hash_bin, sizeof(frontier_hash_bin));
            break;
        default:
            // Get OPEN work
            ESP_LOGI(TAG, "Creating OPEN State Block");
            open = true;
            frontier_block.type = UNDEFINED;
            break;
    }

    /*****************************
     * Create receive/open block *
     *****************************/
    jolt_gui_sem_take();
    jolt_gui_scr_loading_update(scr_loading, NULL, "Creating Block", 30);
    jolt_gui_sem_give();

    nl_block_t receive_block;
    nl_block_init(&receive_block);
    receive_block.type = STATE;
    // Frontier Hash is all zero if its an open
    sodium_hex2bin(receive_block.previous, sizeof(receive_block.previous),
            frontier_hash, sizeof(frontier_hash), NULL, NULL, NULL);
    memcpy(receive_block.account, my_public_key, sizeof(my_public_key));
    nl_address_to_public(receive_block.representative, 
            CONFIG_JOLT_NANO_DEFAULT_REPRESENTATIVE);
    sodium_hex2bin(receive_block.link, sizeof(receive_block.link),
            pending_hash, sizeof(pending_hash), NULL, NULL, NULL);
    mbedtls_mpi_add_abs(&(receive_block.balance), &transaction_amount, &(frontier_block.balance));

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        char amount[66];
        size_t olen;
        mbedtls_mpi_write_string(&(frontier_block.balance), 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "Frontier Amount: %s", amount);
        mbedtls_mpi_write_string(&(receive_block.balance), 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "New Block Amount: %s", amount);
        mbedtls_mpi_write_string(&transaction_amount, 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "Transaction Amount: %s", amount);
    }
    #endif

    /**************************
     * Confirm and Sign Block *
     **************************/
    ESP_LOGI(TAG, "Signing Block");
    if( nano_confirm_block(&frontier_block, &receive_block) ) {
        if( E_SUCCESS != nl_block_sign(&receive_block, my_private_key) ) {
            ESP_LOGI(TAG, "Error Signing Block");
            goto exit;
        }
    }
    else {
        ESP_LOGI(TAG, "User cancelled block during confirmation");
        goto exit;
    }

    /**************
     * Fetch Work *
     **************/
    jolt_gui_sem_take();
    jolt_gui_scr_loading_update(scr_loading, NULL, "Fetching Work", 50);
    jolt_gui_sem_give();

    ESP_LOGI(TAG, "Fetching Work");
    if( open ){
        hex256_t work_hex;
        sodium_bin2hex(work_hex, sizeof(work_hex),
                my_public_key, sizeof(my_public_key));
        if( E_SUCCESS != nanoparse_web_work( work_hex, &proof_of_work ) ){
            ESP_LOGI(TAG, "Invalid Work (OPEN) Response.");
            jolt_gui_sem_take();
            jolt_gui_scr_text_create(TITLE, "Failed Fetching Work");
            jolt_gui_sem_give();
            goto exit;
        }
    }
    else{
        if( E_SUCCESS != nanoparse_web_work( frontier_hash, &proof_of_work ) ){
            ESP_LOGI(TAG, "Invalid Work (RECEIVE) Response.");
            jolt_gui_sem_take();
            jolt_gui_scr_text_create(TITLE, "Failed Fetching Work");
            jolt_gui_sem_give();
            goto exit;
        }
    }
    receive_block.work = proof_of_work;

    /*************
     * Broadcast *
     *************/
    jolt_gui_sem_take();
    jolt_gui_scr_loading_update(scr_loading, NULL, "Broadcasting", 70);
    jolt_gui_sem_give();

    switch(nanoparse_web_process(&receive_block)){
        case E_SUCCESS:
            break;
        default:
            jolt_gui_sem_take();
            jolt_gui_scr_text_create(TITLE, "Error Broadcasting");
            jolt_gui_sem_give();
            goto exit;
    }
    
    jolt_gui_sem_take();
    jolt_gui_scr_text_create(TITLE, "Blocks Processed");
    jolt_gui_sem_give();

    exit:
        jolt_gui_sem_take();
        if( NULL != scr_loading ) {
            lv_obj_del(scr_loading);
        }
        jolt_gui_sem_give();
        vTaskDelete(NULL);
}

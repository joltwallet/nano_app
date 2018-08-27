/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"
#include <string.h>
#include "esp_log.h"

#include "nano_lib.h"
#include "nano_parse.h"
#include "menu8g2.h"

#include "submenus.h"
#include "vault.h"
#include "globals.h"
#include "gui/loading.h"
#include "gui/gui.h"
#include "../nano_helpers.h"
#include "../confirmation.h"

static const char TAG[] = "nano_receive";
static const char TITLE[] = "Receive Nano";


void menu_nano_receive(menu8g2_t *prev){
    /*
     * Blocks involved:
     * pending_block - The send we want to sign in
     * receive_block - block we are creating
     * frontier_block - frontier of our account chain
     */
    menu8g2_t menu_obj;
    menu8g2_t *m = &menu_obj;
    menu8g2_copy(m, prev);
    
    hex256_t pending_hash = { 0 };
    hex256_t frontier_hash = { 0 };

    /***********************************************
     * Get My Public Key, Private Key, and Address *
     ***********************************************/
    CONFIDENTIAL uint256_t my_private_key;
    uint256_t my_public_key;
    char my_address[ADDRESS_BUF_LEN];
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
    loading_enable();
    loading_text_title("Checking Pending", TITLE);
    
    /* Search for pending block(s) */
    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);
    switch(nanoparse_web_pending_hash(my_address, pending_hash, &transaction_amount)){
        case E_SUCCESS:
            break;
        default:
            loading_disable();
            menu8g2_display_text_title(m, "No Pending Blocks Found", TITLE);
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
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, my_public_key, sizeof(my_public_key));
    uint64_t proof_of_work;

    loading_text_title("Checking Account", TITLE);
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
    loading_text_title("Creating Block", TITLE);
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
    loading_disable();
    ESP_LOGI(TAG, "Signing Block");
    if( nano_confirm_block(m, &frontier_block, &receive_block) ) {
        if( E_SUCCESS != nl_block_sign(&receive_block, my_private_key) ) {
            ESP_LOGI(TAG, "Error Signing Block");
            goto exit;
        }
    }
    else {
        ESP_LOGI(TAG, "User cancelled block during confirmation");
        goto exit;
    }
    loading_enable();

    /**************
     * Fetch Work *
     **************/
    loading_text_title("Fetching Work", TITLE);
    ESP_LOGI(TAG, "Fetching Work");
    if( open ){
        hex256_t work_hex;
        sodium_bin2hex(work_hex, sizeof(work_hex),
                my_public_key, sizeof(my_public_key));
        loading_text_title("Fetching Work", TITLE);
        if( E_SUCCESS != nanoparse_web_work( work_hex, &proof_of_work ) ){
            ESP_LOGI(TAG, "Invalid Work (OPEN) Response.");
            loading_disable();
            menu8g2_display_text_title(m, "Failed Fetching Work", TITLE);
            goto exit;
        }
    }
    else{
        if( E_SUCCESS != nanoparse_web_work( frontier_hash, &proof_of_work ) ){
            ESP_LOGI(TAG, "Invalid Work (RECEIVE) Response.");
            loading_disable();
            menu8g2_display_text_title(m, "Failed Fetching Work", TITLE);
            goto exit;
        }
    }
    receive_block.work = proof_of_work;

    /*************
     * Broadcast *
     *************/
    loading_text_title("Broadcasting", TITLE);
    switch(nanoparse_web_process(&receive_block)){
        case E_SUCCESS:
            break;
        default:
            loading_disable();
            menu8g2_display_text_title(m, "Error Broadcasting", TITLE);
            goto exit;
    }
    
    loading_disable();
    menu8g2_display_text_title(m, "Blocks Processed", TITLE);

    exit:
        loading_disable();
        return;
}

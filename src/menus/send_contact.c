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
#include "menu8g2.h"
#include "nano_parse.h"

#include "submenus.h"
#include "../confirmation.h"
#include "../contacts.h"
#include "../nano_helpers.h"
#include "globals.h"
#include "vault.h"
#include "gui/entry.h"
#include "gui/gui.h"
#include "gui/loading.h"

static const char TAG[] = "nano_send_contact";
static const char TITLE[] = "Send Nano";


static void namer(char buf[], size_t buf_len, const char *options[], const uint32_t index){
    if( !nano_get_contact_name(buf, buf_len, index) ){
        strlcpy(buf, "", buf_len);
    }
}
void menu_nano_send_contact(menu8g2_t *prev){
    /*
     * Blocks involved:
     * send_block - block we are creating
     * frontier_block - frontier of our account chain
     */

    menu8g2_t menu_obj;
    menu8g2_t *m = &menu_obj;
    menu8g2_copy(m, prev);

    uint256_t dest_public_key;
    hex256_t frontier_hash = { 0 };
    char dest_amount_buf[40];

    /**************************************
     * Get Destination Address and Amount *
     **************************************/
    /* prompt user to select contact address here */
    if( !menu8g2_create_vertical_menu(m, "Send Contact", NULL,
                        (void *)&namer, CONFIG_JOLT_NANO_CONTACTS_MAX) ){
        ESP_LOGE(TAG, "User Cancelled at Contact Book");
        goto exit;
    }
    {
        uint32_t contact_index = m->index;
        if( !nano_get_contact_public(dest_public_key, contact_index) ){
            ESP_LOGE(TAG, "Contact %d public key doesn't exist.", contact_index);
            goto exit;
        }
    }

    int8_t user_entry[CONFIG_JOLT_NANO_SEND_DIGITS];
    /* user enter send amount in nano */
    if( !entry_number_arr(m, user_entry,
                sizeof(user_entry), CONFIG_JOLT_NANO_SEND_DECIMALS,
                "Enter Amount") ){
        ESP_LOGE(TAG, "User cancelled at amount entry.");
        goto exit;
    }
    uint8_t i;
    for(i = 0; i < sizeof(user_entry); i++){
        dest_amount_buf[i] = user_entry[i] + '0';
    }
    for( uint8_t j = CONFIG_JOLT_NANO_SEND_DECIMALS; j < 30 ; i++, j++ ){
        dest_amount_buf[i] = '0';
    }
    dest_amount_buf[i] = '\0';
    
    mbedtls_mpi transaction_amount;
    mbedtls_mpi_init(&transaction_amount);
    mbedtls_mpi_read_string(&transaction_amount, 10, dest_amount_buf);
    
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

    /***********************************
     * Get My Account's Frontier Block *
     ***********************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, my_public_key, sizeof(my_public_key));
    uint64_t proof_of_work;

    loading_text_title("Checking Account", TITLE);
    ESP_LOGI(TAG, "Fetching Frontier Block Contents");
    switch( nanoparse_web_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            ESP_LOGI(TAG, "Locally recomputing Frontier Hash");
            uint256_t frontier_hash_bin;
            nl_block_compute_hash(&frontier_block, frontier_hash_bin);
            sodium_bin2hex(frontier_hash, sizeof(frontier_hash),
                    frontier_hash_bin, sizeof(frontier_hash_bin));
            break;
        default:
            //To send requires a previous Open Block
            ESP_LOGI(TAG, "Couldn't fetch frontier.");
            loading_disable();
            menu8g2_display_text_title(m, "Could not fetch frontier", TITLE); \
            goto exit;
    }

    /*****************
     * Check Balance *
     *****************/
    if (mbedtls_mpi_cmp_mpi(&(frontier_block.balance), &transaction_amount) == -1) {
        loading_disable();
        ESP_LOGI(TAG, "Insufficent Funds.");
        menu8g2_display_text_title(m, "Insufficent Funds", TITLE);
        goto exit;
    }
    
    /*********************
     * Create send block *
     *********************/
    loading_text_title("Creating Block", TITLE);

    nl_block_t send_block;
    nl_block_init(&send_block);
    send_block.type = STATE;
    sodium_hex2bin(send_block.previous, sizeof(send_block.previous),
            frontier_hash, sizeof(frontier_hash), NULL, NULL, NULL);
    memcpy(send_block.account, my_public_key, sizeof(my_public_key));
    memcpy(send_block.representative, frontier_block.representative, BIN_256);
    memcpy(send_block.link, dest_public_key, sizeof(dest_public_key));
    mbedtls_mpi_sub_abs(&(send_block.balance), &(frontier_block.balance), &transaction_amount);

    #if LOG_LOCAL_LEVEL >= ESP_LOG_INFO
    {
        char amount[66];
        size_t olen;
        mbedtls_mpi_write_string(&(frontier_block.balance), 10, amount, sizeof(amount), &olen);
        ESP_LOGI(TAG, "Frontier Amount: %s", amount);
        mbedtls_mpi_write_string(&(send_block.balance), 10, amount, sizeof(amount), &olen);
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
    if( nano_confirm_block(m, &frontier_block, &send_block) ) {
        if( E_SUCCESS != nl_block_sign(&send_block, my_private_key) ) {
            ESP_LOGI(TAG, "Error Signing Block");
            goto exit;
        }
    }
    else {
        ESP_LOGI(TAG, "User cancelled block during confirmation");
        goto exit;
    }
    loading_enable();

    // Get RECEIVE work
    loading_text_title("Fetching Work", TITLE);
    if( E_SUCCESS != nanoparse_web_work( frontier_hash, &proof_of_work ) ){
        ESP_LOGI(TAG, "Invalid Work (RECEIVE) Response.");
        loading_disable();
        menu8g2_display_text_title(m, "Failed Fetching Work", TITLE); \
        goto exit;
    }
    send_block.work = proof_of_work;

    loading_text_title("Broadcasting", TITLE);
    switch(nanoparse_web_process(&send_block)){
        case E_SUCCESS:
            break;
        default:
            loading_disable();
            menu8g2_display_text_title(m, "Error Broadcasting", TITLE);
            goto exit;
    }

    
    loading_disable();
    menu8g2_display_text_title(m, "Transaction Sent", TITLE);

    exit:
        loading_disable();
        return;
}

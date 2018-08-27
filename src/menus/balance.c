/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "menu8g2.h"
#include "nano_lib.h"
#include "nano_parse.h"
#include "nano_rest.h"
#include "sodium.h"
#include <string.h>

#include "globals.h"
#include "gui/gui.h"
#include "gui/loading.h"
#include "vault.h"

#include "../nano_helpers.h"
#include "submenus.h"

static const char TAG[] = "nano_balance";
static const char TITLE[] = "Nano Balance";


void menu_nano_balance(menu8g2_t *prev){
    /*
     * Blocks involved:
     * frontier_block - frontier of our account chain
     */
    menu8g2_t menu_obj;
    menu8g2_t *m = &menu_obj;
    menu8g2_copy(m, prev);

    double display_amount;

    /*********************
     * Get My Public Key *
     *********************/
    uint256_t my_public_key;
    if( !nano_get_public(my_public_key) ) {
        goto exit;
    }

    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    loading_enable();
    loading_text_title("Getting Frontier", TITLE);

    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, my_public_key, sizeof(my_public_key));

    switch( nanoparse_web_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            ESP_LOGI(TAG, "Successfully fetched frontier block");
            if( E_SUCCESS != nl_mpi_to_nano_double(&(frontier_block.balance),
                        &display_amount) ){
                goto exit;
            }
            ESP_LOGI(TAG, "Approximate Account Balance: %0.3lf", display_amount);
            break;
        default:
            ESP_LOGI(TAG, "Failed to fetch frontier block (does it exist?)");
            display_amount = 0;
            break;
    }

    char buf[100];
    snprintf(buf, sizeof(buf), "%0.3lf Nano", display_amount);

    loading_disable();
    for(;;){
        if(menu8g2_display_text_title(m, buf, TITLE)
                & (1ULL << EASY_INPUT_BACK)){
            goto exit;
        }
    }

    exit:
        loading_disable();
        return;
}

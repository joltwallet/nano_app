/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "../nano_helpers.h"
#include "submenus.h"
#include "nano_parse.h"

static const char TAG[] = "nano_balance";
static const char TITLE[] = "Nano Balance";


lv_action_t menu_nano_balance_cb( lv_obj_t *dummy ) {
    double display_amount;

    lv_obj_t *scr = jolt_gui_scr_loading_create(TITLE);

    /*********************
     * Get My Public Key *
     *********************/
    uint256_t my_public_key;
    if( !nano_get_public(my_public_key) ) {
        return LV_RES_OK;
    }

    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    jolt_gui_scr_loading_update(scr, NULL, "Getting Frontier", 50);

    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, my_public_key, sizeof(my_public_key));

    switch( nanoparse_web_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            ESP_LOGI(TAG, "Successfully fetched frontier block");
            if( E_SUCCESS != nl_mpi_to_nano_double(&(frontier_block.balance),
                        &display_amount) ){
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

    lv_obj_del(scr);
    jolt_gui_scr_text_create(TITLE, buf);

    return LV_RES_OK;
}

lv_action_t menu_nano_balance( lv_obj_t *btn ) {
    vault_refresh(NULL, menu_nano_balance_cb);
    return LV_RES_OK;
}

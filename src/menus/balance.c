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
#include "../nano_network.h"


static const char TAG[] = "nano_balance";
static const char TITLE[] = "Nano Balance";

static void frontier_cb( nl_block_t *block, void *param ) {
    lv_obj_t *scr = param;

    uint256_t my_public_key;
    nano_get_public(my_public_key);
    //assert( 0 == memcmp(my_public_key, block->account, sizeof(uint32_t) ));

    double display_amount;
    if( E_SUCCESS != nl_mpi_to_nano_double(&(block->balance), &display_amount) ){
    }
    ESP_LOGI(TAG, "Approximate Account Balance: %0.3lf", display_amount);

    char buf[100];
    snprintf(buf, sizeof(buf), "%0.3lf Nano", display_amount);

    lv_obj_del(scr);
    jolt_gui_scr_text_create(TITLE, buf);
}

lv_res_t menu_nano_balance_cb( lv_obj_t *dummy ) {
    lv_obj_t *scr = jolt_gui_scr_loading_create(TITLE);
    jolt_gui_scr_loading_update(scr, NULL, "Getting Frontier", 50);

    char address[ADDRESS_BUF_LEN];
    nano_get_address( address );
    nano_network_frontier_block( address, frontier_cb, scr );
    return LV_RES_OK;
}

lv_res_t menu_nano_balance( lv_obj_t *btn ) {
    vault_refresh(NULL, menu_nano_balance_cb);
    return LV_RES_OK;
}

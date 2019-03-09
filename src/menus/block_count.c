/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "jolt_lib.h"
#include "submenus.h"
#include "nano_parse.h"
#include "submenus.h"
#include "../nano_network.h"

static const char TAG[] = "nano_block_count";
static const char TITLE[] = "Block Count";

static void network_cb( uint32_t count, void *param, lv_obj_t *scr ) {
    /* Delete the preloading screen */
    jolt_gui_obj_del( scr );

    /* Create the text screen */
    char block_count[20];
    snprintf(block_count, sizeof(block_count), "%d", count);
    jolt_gui_scr_text_create(TITLE, block_count);
}

lv_res_t menu_nano_block_count(lv_obj_t *btn) {
    lv_obj_t *scr;
    scr = jolt_gui_scr_preloading_create(TITLE, "Connecting To Server");
    if( NULL == scr ) {
        /* Failed to create screen, return early */
        return LV_RES_OK;
    }
    nano_network_block_count(network_cb, NULL, scr);
    return LV_RES_OK;
}

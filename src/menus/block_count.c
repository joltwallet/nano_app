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
    if( count > 0) {
        char block_count[30];
        snprintf(block_count, sizeof(block_count), "Blocks: %d", count);
        jolt_gui_scr_text_create(TITLE, block_count);
    }
    else{
        jolt_gui_scr_text_create(TITLE, "Unable to get block count.");
    }
}

void menu_nano_block_count(lv_obj_t *btn, lv_event_t event) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        lv_obj_t *scr;
        scr = jolt_gui_scr_preloading_create(TITLE, "Connecting To Server");
        if( NULL == scr ) {
            /* Failed to create screen, return early */
            ESP_LOGE(TAG, "Failed to create screen");
            return;
        }
        nano_network_block_count(network_cb, NULL, scr);
    }
}

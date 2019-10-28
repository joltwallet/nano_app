/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "../nano_network.h"
#include "jolt_lib.h"
#include "nano_parse.h"
#include "submenus.h"

static const char TAG[]   = "nano_block_count";
static const char TITLE[] = "Block Count";

static void network_cb( uint32_t count, void *param, jolt_gui_obj_t *scr )
{
    /* Delete the preloading screen */
    jolt_gui_obj_del( scr );

    /* Create the text screen */
    if( count > 0 ) {
        char block_count[30];
        snprintf( block_count, sizeof( block_count ), "Blocks: %d", count );
        jolt_gui_scr_text_create( TITLE, block_count );
    }
    else {
        jolt_gui_scr_text_create( TITLE, "Unable to get block count." );
    }
}

void menu_nano_block_count( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        jolt_gui_obj_t *scr;
        scr = jolt_gui_scr_preloading_create( TITLE, "Connecting To Server" );
        if( NULL == scr ) {
            /* Failed to create screen, return early */
            ESP_LOGE( TAG, "Failed to create screen" );
            return;
        }
        nano_network_block_count( network_cb, NULL, scr );
    }
}

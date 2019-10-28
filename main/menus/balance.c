/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "../nano_helpers.h"
#include "../nano_network.h"
#include "esp_log.h"
#include "jolt_lib.h"
#include "nano_lib.h"
#include "nano_parse.h"
#include "submenus.h"

static const char TAG[]   = "nano_balance";
static const char TITLE[] = "Nano Balance";

static void frontier_cb( nl_block_t *block, void *param, jolt_gui_obj_t *scr )
{
    {
        uint256_t my_public_key;
        nano_get_public( my_public_key );
        // assert( 0 == memcmp(my_public_key, block->account, sizeof(uint32_t) ));
    }

    jolt_gui_obj_del( scr );
    if( NULL == block ) {
        jolt_gui_scr_text_create( TITLE, "Unable to get account info." );
        return;
    }

    double display_amount;
    if( E_SUCCESS != nl_mpi_to_nano_double( &( block->balance ), &display_amount ) ) {
        // todo: error
    }
    ESP_LOGI( TAG, "Approximate Account Balance: %0.3lf", display_amount );

    char buf[100];
    snprintf( buf, sizeof( buf ), "%0.3lf Nano", display_amount );

    jolt_gui_scr_text_create( TITLE, buf );
}

void menu_nano_balance_cb( void *dummy )
{
    jolt_gui_obj_t *scr = jolt_gui_scr_preloading_create( TITLE, "Contacting Server" );

    char address[ADDRESS_BUF_LEN];
    nano_get_address( address );
    nano_network_frontier_block( address, frontier_cb, NULL, scr );
}

void menu_nano_balance( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) { vault_refresh( NULL, menu_nano_balance_cb, NULL ); }
}

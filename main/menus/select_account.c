/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "../nano_helpers.h"
#include "esp_log.h"
#include "jolt_lib.h"
#include "nano_lib.h"
#include "submenus.h"

static const char TAG[] = "nano_sel_acc";

/* Stores the selected index to nano_index */
static void menu_nano_select_account_index_cb( jolt_gui_obj_t *btn_sel, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        int32_t index = jolt_gui_scr_menu_get_btn_index( btn_sel );
        if( index >= 0 ) {
            ESP_LOGI( TAG, "Saving index %d", index );
            nano_index_set( NULL, index );
        }
        else {
            ESP_LOGE( TAG, "Selected button not found in list" );
        }
        jolt_gui_scr_del( btn_sel );
    }
}

static void menu_nano_select_account_cb( void *dummy )
{
    const char title[] = "Nano";

    uint32_t index = nano_index_get( NULL );
    ESP_LOGI( TAG, "Current Nano Address Derivation Index: %d", index );

    jolt_gui_obj_t *menu = jolt_gui_scr_menu_create( title );
    jolt_gui_obj_t *sel  = NULL;

    for( uint8_t i = 0; i < 10; i++ ) {
        char address[ADDRESS_BUF_LEN];
        char buf[ADDRESS_BUF_LEN + 16];
        if( !nano_index_get_address( address, i ) ) { strlcpy( buf, "ERROR", sizeof( buf ) ); }
        else {
            snprintf( buf, sizeof( buf ), "%d. %s", i + 1, address );
        }
        jolt_gui_obj_t *btn = jolt_gui_scr_menu_add( menu, NULL, buf, menu_nano_select_account_index_cb );
        if( i == index || 0 == i ) { sel = btn; }
    }
    jolt_gui_scr_menu_set_btn_selected( menu, sel );
}

void menu_nano_select_account( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) { vault_refresh( NULL, menu_nano_select_account_cb, NULL ); }
}

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "../nano_helpers.h"
#include "../qr_helpers.h"
#include "esp_log.h"
#include "jolt_lib.h"
#include "nano_lib.h"
#include "submenus.h"

static const char *TAG    = __FILE__;
static const char TITLE[] = "Nano Address";

static void menu_nano_address_cb( void *dummy )
{
    char buf[120];  // shared buffer for text-address and QR data
    uint256_t public_key;
    jolt_gui_obj_t *scr = NULL;
    jolt_err_t err;

    if( !nano_get_private_public_address( NULL, public_key, buf ) ) { goto exit; }
    else {
        ESP_LOGI( TAG, "Address: %s", buf );
    }

    scr = jolt_gui_scr_scroll_create( TITLE );
    if( NULL == scr ) { goto exit; }

    jolt_gui_scr_scroll_add_monospace_text( scr,
                                            buf );  // todo; break up into 4 lines

    /* Generate QR code */
    err = receive_url_create( buf, sizeof( buf ), public_key, NULL );
    if( err != E_SUCCESS ) { goto exit; }

    jolt_gui_scr_scroll_add_qr( scr, buf, strlen( buf ) );

    return;

exit:
    if( NULL != scr ) { jolt_gui_obj_del( scr ); }
}

void menu_nano_address( jolt_gui_obj_t *btn, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) { vault_refresh( NULL, menu_nano_address_cb, NULL ); }
}

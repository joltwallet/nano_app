/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "../nano_helpers.h"
#include "submenus.h"

static const char* TAG = "nano_add_text";
static const char TITLE[] = "Nano Address";


static lv_res_t menu_nano_address_text_cb( lv_obj_t *dummy ) {
    char address[ADDRESS_BUF_LEN];
    if( !nano_get_address(address) ) {
        return;
    }
    ESP_LOGI(TAG, "Address: %s", address);
    jolt_gui_scr_text_create(TITLE, address);

    return LV_RES_OK;
}

lv_res_t menu_nano_address_text( lv_obj_t *btn ) {
    vault_refresh(NULL, menu_nano_address_text_cb);
    return LV_RES_OK;
}

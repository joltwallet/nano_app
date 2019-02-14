/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "../nano_helpers.h"
#include "../qr_helpers.h"
#include "submenus.h"

static const char TAG[] = "nano_qr";

lv_res_t menu_nano_address_qr_cb(lv_obj_t *dummy) {
    char buf[120];
    uint256_t public_key;
    jolt_err_t err;

    ESP_LOGI(TAG, "Computing Public Key");
    if( !nano_get_public(public_key) ) {
        return LV_RES_OK;
    }

    ESP_LOGI(TAG, "Generating QR Code");
    err = receive_url_create(buf, sizeof(buf), public_key, NULL);
    if( err != E_SUCCESS ) {
        return LV_RES_OK;
    }

    jolt_gui_scr_qr_create("Address", buf, strlen(buf));

    return LV_RES_OK;
}

lv_res_t menu_nano_address_qr(lv_obj_t *btn) {
    vault_refresh(NULL, menu_nano_address_qr_cb);
    return LV_RES_OK;
}

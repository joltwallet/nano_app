/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "../nano_helpers.h"
#include "submenus.h"

static const char TAG[] = "nano_sel_acc";
static const char TITLE[] = "Nano Account";

/* Stores the selected index to nano_index */
static void menu_nano_select_account_index_cb( lv_obj_t *btn_sel, lv_event_t event ) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        int32_t index = jolt_gui_scr_menu_get_btn_index( btn_sel );
        if( index >= 0 ) {
            ESP_LOGI(TAG, "Saving index %d", index);
            nano_index_set(NULL, index);
        }
        else {
            ESP_LOGE(TAG, "Selected button not found in list");
        }
        jolt_gui_scr_del();
    }
}

static void menu_nano_select_account_cb( void *dummy ) {
    const char title[] = "Nano";

    uint32_t index = nano_index_get(NULL);
    ESP_LOGI(TAG, "Current Nano Address Derivation Index: %d", index);

    lv_obj_t *menu = jolt_gui_scr_menu_create(title);
    lv_obj_t *sel = NULL;

    for(uint8_t i=0; i < 10; i++) {
        char address[ADDRESS_BUF_LEN];
        char buf[ADDRESS_BUF_LEN+16];
        if( !nano_index_get_address(address, i) ) {
            strlcpy(buf, "ERROR", sizeof(buf));
        }
        else {
            snprintf(buf, sizeof(buf), "%d. %s", i+1, address);
        }
        lv_obj_t *btn = jolt_gui_scr_menu_add(menu, NULL, buf,
                menu_nano_select_account_index_cb);
        if( i == index || 0 == i ) {
            sel = btn;
        }
    }
    jolt_gui_scr_menu_set_btn_selected(menu, sel);
}

void menu_nano_select_account( lv_obj_t *btn, lv_event_t event ) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        vault_refresh(NULL, menu_nano_select_account_cb, NULL);
    }
}

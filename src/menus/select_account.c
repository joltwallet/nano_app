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
static lv_action_t menu_nano_select_account_index_cb( lv_obj_t *btn_sel ) {
    lv_obj_t *list = lv_obj_get_parent(lv_obj_get_parent( btn_sel ));
    int32_t index = lv_list_get_btn_index(list, btn_sel);
    if( index >= 0 ) {
        ESP_LOGI(TAG, "Saving index %d", index);
        nano_index_set(index);
    }
    else {
        ESP_LOGE(TAG, "Selected button not found in list");
    }
    return LV_RES_OK;
}

static lv_action_t menu_nano_select_account_cb( lv_obj_t *btn ) {
    const char title[] = "Nano";

    uint32_t index = nano_index_get();
    ESP_LOGD(TAG, "Current Nano Address Derivation Index: %d", index);

    lv_obj_t *menu = jolt_gui_scr_menu_create(title);
    lv_obj_t *list = jolt_gui_scr_menu_get_list(menu);

    ESP_LOGD(TAG, "Account Select List: %p", list);
    lv_obj_t *sel;

    for(uint8_t i=0; i < 10; i++) {
        char address[ADDRESS_BUF_LEN];
        char buf[20];
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
    lv_list_set_btn_selected(list, sel);

    return LV_RES_OK;
}

lv_action_t menu_nano_select_account( lv_obj_t *btn ) {
    vault_refresh(NULL, menu_nano_select_account_cb);
    return LV_RES_OK;
}

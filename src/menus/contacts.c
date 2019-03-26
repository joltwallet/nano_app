/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "nano_helpers.h"
#include "submenus.h"
#include "nano_parse.h"
#include "nano_network.h"

/*
 * Contacts fields:
 *     * Name
 *     * Address
 */

static const char TAG[] = "nano_contacts";
static const char TITLE[] = "Contacts";

static lv_res_t contact_cb(lv_obj_t *btn) {
    return LV_RES_OK;
}

lv_res_t menu_nano_contacts( lv_obj_t *btn ) {
    /* Read Config */
    lv_obj_t *menu = NULL;
    cJSON *contact = NULL;
    cJSON *json = nano_get_json();
    cJSON *contacts = cJSON_Get(json, "contacts");

    int n_contacts = cJSON_GetArraySize(contacts);

    if( 0 == n_contacts ) {
        jolt_gui_scr_text_create(TITLE, "No Contacts");
        goto exit;
    }

    ESP_LOGD(TAG, "Creating contacts menu");
    menu = jolt_gui_scr_menu_create(TITLE);

    ESP_LOGD(TAG, "Iterating through saved contacts");
    cJSON_ArrayForEach(contact, contacts){
        cJSON *elem;
        elem = cJSON_Get(contact, "name");
        ESP_LOGD(TAG, "Adding contact list element-name \"%s\"", cJSON_GetStringValue(elem));
        jolt_gui_scr_menu_add(menu, NULL, cJSON_GetStringValue(elem), contact_cb);
    }

exit:
    jolt_json_del(json);
    return LV_RES_OK;
}

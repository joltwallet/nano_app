/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"
#include "esp_log.h"

#include "nano_lib.h"
#include "menu8g2.h"
#include "submenus.h"
#include "../nano_helpers.h"

static const char* TAG = "nano_add_text";
static const char TITLE[] = "Nano Address";

void menu_nano_address_text(menu8g2_t *prev){
    char address[ADDRESS_BUF_LEN];
    if( !nano_get_address(address) ) {
        return;
    }
    ESP_LOGI(TAG, "Address: %s", address);
    for(;;){
        if(menu8g2_display_text_title(prev, address, TITLE)
                & (1ULL << EASY_INPUT_BACK)){
            return;
        }
    }
}

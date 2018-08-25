/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"

#include "nano_lib.h"
#include "qrcode.h"
#include "esp_log.h"

#include "../nano_helpers.h"
#include "menu8g2.h"
#include "submenus.h"
#include "helpers.h"
#include "gui/qr.h"
#include "gui/gui.h"
#include "hal/storage.h"
#include "../qr_helper.h"

const char TAG[] = "nano_qr";


void menu_nano_address_qr(menu8g2_t *prev){
    QRCode qrcode;
    uint8_t qrcode_bytes[qrcode_getBufferSize(CONFIG_JOLT_QR_VERSION)];
    uint64_t input_buf;
    uint256_t public_key;
    jolt_err_t err;

    ESP_LOGI(TAG, "Computing Public Key");
    if( !nano_get_public(public_key) ) {
        return;
    }

    ESP_LOGI(TAG, "Generating QR Code");
    err = public_to_qr(&qrcode, qrcode_bytes, public_key, NULL); 
    if( err != E_SUCCESS ) {
        goto exit;
    }

    //FULLSCREEN_ENTER;

    ESP_LOGI(TAG, "Dimming Display");
    SCREEN_MUTEX_TAKE;
    u8g2_SetContrast(prev->u8g2, 1); // Phones have trouble with bright displays
    SCREEN_MUTEX_GIVE;
    ESP_LOGI(TAG, "Drawing QR Code");
    display_qr_center(prev, &qrcode, CONFIG_JOLT_QR_SCALE);

    for(;;){
        if(xQueueReceive(prev->input_queue, &input_buf, portMAX_DELAY)) {
            if(input_buf & (1ULL << EASY_INPUT_BACK)){
                // Restore User's Brightness
                SCREEN_MUTEX_TAKE;
                u8g2_SetContrast(prev->u8g2, get_display_brightness());
                SCREEN_MUTEX_GIVE;

                //FULLSCREEN_EXIT;
                goto exit;
            }
        }
    }

    exit:
        return;
}

/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "sodium.h"
#include <string.h>
#include "esp_log.h"

#include "nano_lib.h"
#include "menu8g2.h"
#include "nano_parse.h"

#include "submenus.h"
#include "vault.h"
#include "globals.h"
#include "gui/gui.h"
#include "gui/loading.h"
#include "../nano_helpers.h"

static const char TAG[] = "nano_sel_acc";
static const char TITLE[] = "Nano Account";


static menu8g2_err_t get_nano_address(char buf[], size_t buf_len, const char *options[], const uint32_t index){
    char address[ADDRESS_BUF_LEN];
    if( !nano_index_get_address(address, index) ) {
        strlcpy(buf, "ERROR", buf_len);
        return MENU8G2_FAILURE;
    }
    else{
        snprintf(buf, buf_len, "%d. %s", index, address);
        return MENU8G2_SUCCESS;
    }
}

void menu_nano_select_account(menu8g2_t *prev){
    menu8g2_t menu_obj;
    menu8g2_t *m = & menu_obj;
    menu8g2_copy(m, prev);

    vault_refresh();
    m->index = nano_index_get();

    if(menu8g2_create_vertical_menu(m, TITLE, NULL,
            (void *)&get_nano_address, UINT32_MAX)){
        // Enter
        nano_index_set(m->index);
    }
}

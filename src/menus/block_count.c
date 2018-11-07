/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "jolt_lib.h"
#include "submenus.h"
#include "nano_parse.h"
#include "submenus.h"

static const char TITLE[] = "Block Count";

lv_action_t menu_nano_block_count(lv_obj_t *btn) {
    char block_count[20];
    uint32_t count = nanoparse_web_block_count();
    if( 0 == count ) {
        jolt_gui_scr_text_create(TITLE, "Couldn't contact server."); // todo; JoltOS generic messages
    }
    else {
        sprintf(block_count, "%d", nanoparse_web_block_count());
        jolt_gui_scr_text_create(TITLE, block_count);
    }
    return LV_RES_OK;
}


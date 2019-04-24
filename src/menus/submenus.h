/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_NANO_SUBMENUS_H__
#define __JOLT_NANO_SUBMENUS_H__

#include "lvgl/lvgl.h"
#include "sdkconfig.h"

void menu_nano_block_count(lv_obj_t *btn, lv_event_t event);
void menu_nano_address(lv_obj_t *btn, lv_event_t event);
void menu_nano_contacts(lv_obj_t *btn, lv_event_t event);
void menu_nano_select_account(lv_obj_t *btn, lv_event_t event);
void menu_nano_balance(lv_obj_t *btn, lv_event_t event);
void menu_nano_receive(lv_obj_t *btn, lv_event_t event );

#endif

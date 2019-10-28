/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_NANO_SUBMENUS_H__
#define __JOLT_NANO_SUBMENUS_H__

#include "lvgl/lvgl.h"
#include "sdkconfig.h"

void menu_nano_block_count( jolt_gui_obj_t *btn, jolt_gui_event_t event );
void menu_nano_address( jolt_gui_obj_t *btn, jolt_gui_event_t event );
void menu_nano_contacts( jolt_gui_obj_t *btn, jolt_gui_event_t event );
void menu_nano_select_account( jolt_gui_obj_t *btn, jolt_gui_event_t event );
void menu_nano_balance( jolt_gui_obj_t *btn, jolt_gui_event_t event );
void menu_nano_receive( jolt_gui_obj_t *btn, jolt_gui_event_t event );

#endif

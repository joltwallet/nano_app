/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#ifndef __JOLT_NANO_CONFIRMATION_H__
#define __JOLT_NANO_CONFIRMATION_H__

#include "jolt_lib.h"
#include "nano_lib.h"

typedef void ( *confirm_cb_t )( bool confirm, void *param );

void nano_confirm_block( nl_block_t *head_block, nl_block_t *new_block, confirm_cb_t cb, void *param );

#if 0
bool nano_confirm_contact_update(const menu8g2_t *prev_menu, const char *name,
        const uint256_t public, const uint8_t index);
#endif

#ifndef CONFIG_JOLT_NANO_CONFIRM_DECIMALS
    #define CONFIG_JOLT_NANO_CONFIRM_DECIMALS 3
#endif

#endif

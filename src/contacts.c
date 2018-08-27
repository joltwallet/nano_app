/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <esp_system.h>
#include "esp_log.h"

#include "u8g2.h"
#include "menu8g2.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "nano_lib.h"
#include "vault.h"
#include "hal/storage.h"

#define STORAGE_NAMESPACE "nano"


bool nano_set_contact_public(const uint256_t public_key, const int index){
    /* Returns false on fetch failure */
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    char key[16];
    snprintf(key, sizeof(key), "contact_pub_%d", index);
    storage_set_blob(public_key, BIN_256, STORAGE_NAMESPACE, key);
    return true;
}

bool nano_set_contact_name(const char *buf, const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    if(strlen(buf)>CONFIG_JOLT_NANO_CONTACTS_NAME_LEN){
        return false;
    }

    char key[16];
    snprintf(key, sizeof(key), "contact_name_%d", index);

    storage_set_str(buf, STORAGE_NAMESPACE, key);

    return true;
}

bool nano_get_contact_public(uint256_t public_key, const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    char key[16];
    snprintf(key, sizeof(key), "contact_pub_%d", index);
    size_t size;
    // todo: add size check
    storage_get_blob(public_key, &size, STORAGE_NAMESPACE, key);
    return true;
}

bool nano_get_contact_name(char *buf, size_t buf_len, const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }
    char key[16];
    snprintf(key, sizeof(key), "contact_name_%d", index);

    size_t size;
    storage_get_str(NULL, &size, STORAGE_NAMESPACE, key, ""); 

    if(size > buf_len){
        return false;
    }

    if( size>buf_len || !storage_get_str(NULL, &size, STORAGE_NAMESPACE, key, "") ) {
        return false;
    }

    return true;
}

void nano_erase_contact(const int index){
    if(index >= CONFIG_JOLT_NANO_CONTACTS_MAX){
        return false;
    }

    char key[16];
    snprintf(key, sizeof(key), "contact_pub_%d", index);
    storage_erase_key(STORAGE_NAMESPACE, key);
    snprintf(key, sizeof(key), "contact_name_%d", index);
    storage_erase_key(STORAGE_NAMESPACE, key);
}

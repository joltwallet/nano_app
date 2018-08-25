
#include <stddef.h>
#include <string.h>
#include "nano_lib.h"
#include "vault.h"
#include "hal/storage.h"
#include "sodium.h"
#include "globals.h"


bool nano_get_private(uint256_t private_key) {
    uint32_t index;
    CONFIDENTIAL hd_node_t node;

    if ( !vault_refresh() ) {
        return false;
    }

    vault_sem_take();
    hd_node_copy(&node, &vault->node);
    vault_sem_give();

    storage_get_u32(&index, "nano", "index", 0);
    hd_node_iterate(&node, index);
    memcpy(private_key, &node.key, sizeof(uint256_t));
    sodium_memzero(&node, sizeof(node));
    return true;
}

bool nano_get_private_public(uint256_t private_key, uint256_t public_key) {
    /* Populates private_key/public_key based on storage index.
     * Will not copy to NULL pointers */
    bool res;
    CONFIDENTIAL uint256_t private_key_local;
    if( private_key || public_key ) {
        if( !nano_get_private(private_key_local) ) {
            return false;
        }
        if( private_key ) {
            memcpy(private_key, private_key_local, sizeof(private_key_local));
        }
    }
    if( public_key ) {
        nl_private_to_public(public_key, private_key_local);
    }
    sodium_memzero(private_key_local, sizeof(private_key_local));
    return true;
}

bool nano_get_public(uint256_t public_key) {
    return nano_get_private_public(NULL, public_key);
}

bool nano_get_private_public_address(uint256_t private_key, uint256_t public_key, char *address) {
    /* Assumes address buffer is big enough */
    bool res = false;
    CONFIDENTIAL uint256_t private_key_local;
    uint256_t public_key_local;
    if( !nano_get_private_public(private_key_local, public_key_local) ) {
        goto err;
    }
    if( private_key ) {
        memcpy(private_key, private_key_local, sizeof(private_key_local));
    }
    if( public_key ) {
        memcpy(public_key, public_key_local, sizeof(public_key_local));
    }
    if( address ) {
        nl_public_to_address(address, ADDRESS_BUF_LEN, public_key_local);
    }
    res = true;
err:
    sodium_memzero(private_key_local, sizeof(private_key_local));
    return res;
}

bool nano_get_address(char *address) {
    return nano_get_private_public_address(NULL, NULL, address);
}

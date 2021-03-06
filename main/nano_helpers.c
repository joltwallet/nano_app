
#include "nano_helpers.h"
#include <stddef.h>
#include <string.h>
#include "jolt_lib.h"
#include "nano_lib.h"

static const char TAG[] = "nano_helpers";

/**
 * @brief Ensure all the fields are present in the json config.
 */
static cJSON *verify_json( cJSON *json )
{
    bool save = false;
    if( NULL == json ) {
        /* Create Default JSON */
        json = cJSON_CreateObject();
    }

    if( NULL == cJSON_Get( json, "index" ) ) {
        save = true;
        ESP_LOGW( TAG, "key %s not found", "index" );
        EXIT_IF_NULL( cJSON_AddNumberToObject( json, "index", 0 ) );
    }
    if( NULL == cJSON_Get( json, "contacts" ) ) {
        save = true;
        ESP_LOGW( TAG, "key %s not found", "index" );
        EXIT_IF_NULL( cJSON_AddArrayToObject( json, "contacts" ) );
    }
    if( save ) { jolt_json_write_app( json ); }
    return json;
exit:
    assert( false );  // force a reboot
    jolt_json_del( json );
    return NULL;
}

cJSON *nano_get_json()
{
    cJSON *json = jolt_json_read_app();
    return verify_json( json );
}

uint32_t nano_index_get( cJSON *json )
{
    uint32_t index = 0;
    cJSON *obj;
    bool del_json = false;  // JSON object needs to be freed

    if( NULL == json ) {
        json     = nano_get_json();
        del_json = true;
    }
    obj = cJSON_GetObjectItemCaseSensitive( json, "index" );
    if( NULL == obj ) {
        ESP_LOGW( TAG, "\"index\" key doesn't exist" );
        return 0;
    }
    else {
        index = obj->valuedouble;
    }

    if( del_json ) { cJSON_Delete( json ); }

    return index;
}

bool nano_index_set( cJSON *json, uint32_t index )
{
    cJSON *obj;
    bool del_json = false;

    if( NULL == json ) {
        json     = nano_get_json();
        del_json = true;
    }
    obj = cJSON_GetObjectItemCaseSensitive( json, "index" );
    if( NULL == obj ) { ESP_LOGW( TAG, "\"index\" key doesn't exist" ); }
    else {
        cJSON_SetNumberValue( obj, index );
    }

    jolt_json_write_app( json );
    if( del_json ) { cJSON_Delete( json ); }

    return true;
}

/* Assumes thaat the vault has been externally refreshed */
bool nano_index_get_private( uint256_t private_key, const uint32_t index )
{
    CONFIDENTIAL hd_node_t node;

    vault_sem_take();
    if( vault_get_valid() ) { hd_node_copy( &node, vault_get_node() ); }
    else {
        vault_sem_give();
        return false;
    }
    vault_sem_give();

    hd_node_iterate( &node, index | HARDEN );
    memcpy( private_key, &node.key, sizeof( uint256_t ) );

    sodium_memzero( &node, sizeof( node ) );
    return true;
}

bool nano_index_get_private_public( uint256_t private_key, uint256_t public_key, const uint32_t index )
{
    /* Populates private_key/public_key at given index.
     * Will not copy to NULL pointers */
    CONFIDENTIAL uint256_t private_key_local;
    if( private_key || public_key ) {
        if( !nano_index_get_private( private_key_local, index ) ) { return false; }
        if( private_key ) { memcpy( private_key, private_key_local, sizeof( private_key_local ) ); }
    }
    if( public_key ) { nl_private_to_public( public_key, private_key_local ); }
    sodium_memzero( private_key_local, sizeof( private_key_local ) );
    return true;
}

bool nano_index_get_public( uint256_t public_key, const uint32_t index )
{
    return nano_index_get_private_public( NULL, public_key, index );
}

bool nano_index_get_private_public_address( uint256_t private_key, uint256_t public_key, char *address,
                                            const uint32_t index )
{
    /* Assumes address buffer is big enough */
    bool res = false;
    CONFIDENTIAL uint256_t private_key_local;
    uint256_t public_key_local;
    if( !nano_index_get_private_public( private_key_local, public_key_local, index ) ) { goto err; }
    if( private_key ) { memcpy( private_key, private_key_local, sizeof( private_key_local ) ); }
    if( public_key ) { memcpy( public_key, public_key_local, sizeof( public_key_local ) ); }
    if( address ) { nl_public_to_address( address, ADDRESS_BUF_LEN, public_key_local ); }
    res = true;
err:
    sodium_memzero( private_key_local, sizeof( private_key_local ) );
    return res;
}

bool nano_index_get_address( char *address, const uint32_t index )
{
    return nano_index_get_private_public_address( NULL, NULL, address, index );
}

bool nano_get_private( uint256_t private_key )
{
    uint32_t index = nano_index_get( NULL );
    return nano_index_get_private( private_key, index );
}

bool nano_get_private_public( uint256_t private_key, uint256_t public_key )
{
    /* Populates private_key/public_key based on storage index.
     * Will not copy to NULL pointers */
    uint32_t index = nano_index_get( NULL );
    return nano_index_get_private_public( private_key, public_key, index );
}

bool nano_get_public( uint256_t public_key ) { return nano_get_private_public( NULL, public_key ); }

bool nano_get_private_public_address( uint256_t private_key, uint256_t public_key, char *address )
{
    uint32_t index = nano_index_get( NULL );
    return nano_index_get_private_public_address( private_key, public_key, address, index );
}

bool nano_get_address( char *address ) { return nano_get_private_public_address( NULL, NULL, address ); }

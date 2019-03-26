#ifndef __JOLT_NANO_HELPERS_H__
#define __JOLT_NANO_HELPERS_H__

#include <stdbool.h>
#include "jolttypes.h"
#include "cJSON.h"

#define MAX_CONTACT_NAME_LEN 32 /**< Maximum Contact Name length */
#define MAX_CONTACT_LEN 10      /**< Maximum number of contacts */
#define HARDEN 0x80000000
#define cJSON_Get cJSON_GetObjectItemCaseSensitive

cJSON *nano_get_json();

uint32_t nano_index_get();
bool nano_index_set(uint32_t index);

bool nano_index_get_private(uint256_t private_key, const uint32_t index);
bool nano_index_get_private_public(uint256_t private_key, uint256_t public_key, const uint32_t index);
bool nano_index_get_public(uint256_t public_key, const uint32_t index);
bool nano_index_get_private_public_address(uint256_t private_key, uint256_t public_key, char *address, const uint32_t index);
bool nano_index_get_address(char *address, const uint32_t index);

bool nano_get_private(uint256_t private_key);
bool nano_get_private_public(uint256_t private_key, uint256_t public_key);
bool nano_get_public(uint256_t public_key);
bool nano_get_private_public_address(uint256_t private_key, uint256_t public_key, char *address);
bool nano_get_address(char *address);

#endif

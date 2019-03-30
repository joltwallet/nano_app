#ifndef __JOLT_NANO_HELPERS_H__
#define __JOLT_NANO_HELPERS_H__

#include <stdbool.h>
#include "jolttypes.h"
#include "cJSON.h"

#define HARDEN 0x80000000
#define cJSON_Get cJSON_GetObjectItemCaseSensitive

/**
 * @brief Loads and verifies the application configuration JSON
 * @return Config json
 */
cJSON *nano_get_json();

/**
 * @brief Gets the currently saved account index.
 * @param[in] json Optional input if json has been previously parsed
 * @return Account index; defaults/errors to 0.
 */
uint32_t nano_index_get(cJSON *json);

/**
 * @brief Sets and saved the provided account index
 * @param[in] json Optional input if json has been previously parsed
 * @param[in] index Index to save
 * @return true on success
 */
bool nano_index_set(cJSON *json, uint32_t index);

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

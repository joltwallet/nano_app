#ifndef __JOLT_NANO_HELPERS_H__
#define __JOLT_NANO_HELPERS_H__

#include <stdbool.h>
#include "jolttypes.h"

bool nano_get_private(uint256_t private_key);
bool nano_get_private_public(uint256_t private_key, uint256_t public_key);
bool nano_get_public(uint256_t public_key);
bool nano_get_private_public_address(uint256_t private_key, uint256_t public_key, char *address);
bool nano_get_address(char *address);

#endif

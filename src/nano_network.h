#ifndef JOLT_NANO_NETWORK_H__
#define JOLT_NANO_NETWORK_H__

#include "jolttypes.h"
#include "nano_lib.h"

typedef void (*nano_network_block_count_cb_t)( uint32_t count, void *param );
typedef void (*nano_network_work_cb_t)( uint64_t work, void *param );
typedef void (*nano_network_account_frontier_cb_t)( hex256_t frontier_block_hash, void *param );
typedef void (*nano_network_block_cb_t)( nl_block_t *block, void *param );
typedef void (*nano_network_pending_hash_cb_t)(hex256_t pending_block_hash, mbedtls_mpi *amount, void *param);
typedef nano_network_block_cb_t nano_network_frontier_block_cb_t;
typedef void (*nano_network_process_cb_t)( esp_err_t status, void *param);


esp_err_t nano_network_block_count(nano_network_block_count_cb_t cb, void *param);
esp_err_t nano_network_work( const hex256_t hash, nano_network_work_cb_t cb, void *param );
esp_err_t nano_network_account_frontier( const char *account_address, nano_network_account_frontier_cb_t cb, void *param );
esp_err_t nano_network_block( const hex256_t block_hash, nano_network_block_cb_t cb, void *param );
esp_err_t nano_network_pending_hash( const char *account_address, nano_network_pending_hash_cb_t cb, void *param );
esp_err_t nano_network_frontier_block( const char *address, nano_network_frontier_block_cb_t cb, void *param );
esp_err_t nano_network_process( const nl_block_t *block, nano_network_process_cb_t cb, void *param);


#endif

/**
 * @file nano_network.h
 * @brief Functions to interface with the Nano network
 * @author Brian Pugh
 *
 * @bugs
 *     * nano_network_process doesn't perform error handling (yet)
 */
#ifndef JOLT_NANO_NETWORK_H__
#define JOLT_NANO_NETWORK_H__

#include "jolttypes.h"
#include "nano_lib.h"
#include "jolt_lib.h"

typedef void (*nano_network_block_count_cb_t)( uint32_t count, void *param, lv_obj_t *scr );
typedef void (*nano_network_work_cb_t)( uint64_t work, void *param, lv_obj_t *scr );
typedef void (*nano_network_frontier_hash_cb_t)( hex256_t frontier_block_hash, void *param, lv_obj_t *scr );
typedef void (*nano_network_block_cb_t)( nl_block_t *block, void *param, lv_obj_t *scr );
typedef void (*nano_network_pending_hash_cb_t)(hex256_t pending_block_hash, mbedtls_mpi *amount, void *param, lv_obj_t *scr);
typedef nano_network_block_cb_t nano_network_frontier_block_cb_t;
typedef void (*nano_network_process_cb_t)( esp_err_t status, void *param, lv_obj_t *scr);


/**
 * @brief Get the Nano Network Block Count.
 *
 * Will populate a "0" count for the callback on error.
 *
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_block_count(nano_network_block_count_cb_t cb, void *param, lv_obj_t *scr);

/**
 * @brief Request the Server for work on a given hash (hex).
 *
 * Will populate a "0" work nonce for the callback on failure. 
 * Technically, 0 could be a valid PoW, but we will use it as an error value.
 *
 * @param[in] hash Hash in ascii hex to compute Proof of Work for
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_work( const hex256_t hash, nano_network_work_cb_t cb, void *param, lv_obj_t *scr );

/**
 * @brief Request the Server for work on a given hash (binary).
 * @param[in] hash Hash in ascii hex to compute Proof of Work for
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_work_bin( const uint256_t hash_bin, nano_network_work_cb_t cb, void *param, lv_obj_t *scr );

/**
 * @brief Get the head block hash for some Nano account.
 *
 * Will populate a NULL hash pointer for the callback on failure. 
 *
 * @param[in] account_address Nano account starting with "xrb_" or "nano_"
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_frontier_hash( const char *account_address, nano_network_frontier_hash_cb_t cb, void *param, lv_obj_t *scr );

/**
 * @brief Fetch the block for the specified hash.
 *
 * Will populate a NULL block pointer for the callback on error.
 *
 * @param[in] block_hash Hash of block to fetch contents of.
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_block( const hex256_t block_hash, nano_network_block_cb_t cb, void *param, lv_obj_t *scr );

/**
 * @brief Get the first pending block for an account
 *
 * Will populate NULL pointers for hash and amount for the callback on error.
 *
 * @param[in] account_address Address starting with "xrb_" or "nano_"
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_pending_hash( const char *account_address, nano_network_pending_hash_cb_t cb, void *param, lv_obj_t *scr );

/**
 * @brief Get the frontier block of an account
 *
 * This is a convenience function to get frontier hash and block contents.
 * Fills in block's field according to account in block->account.
 *
 * General Steps:
 *     1) Get Frontier Block Hash
 *     2) Query that hash
 *     3) Call the User cb
 *
 * Will populate a NULL block pointer for the callback on error.
 *
 * @param[in] account_address Address starting with "xrb_" or "nano_"
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_frontier_block( const char *address, nano_network_frontier_block_cb_t cb, void *param, lv_obj_t *scr );

/**
 * @brief Send a signed block off to the network.
 * @param[in] block Signed block to process
 * @param[in] cb Callback to be executed afterwards
 * @param[in] param Pointer to be passed to callback
 */
esp_err_t nano_network_process( const nl_block_t *block, nano_network_process_cb_t cb, void *param, lv_obj_t *scr);


#endif

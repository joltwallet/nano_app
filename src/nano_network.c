/* nano_lib - ESP32 Any functions related to seed/private keys for Nano
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

/* In general, these functions will call the user callback when data is 
 * available or an error occurs. If an error occurs, appropriate sentinel args
 * will be provided (generally 0 or NULL). If a pointer is provided to the 
 * user callback as an argument, the memory it is pointing to is in the heap
 * and it is the callback's responsibility to free the memory.
 */

#include "jolt_lib.h"
#include "nano_parse.h"
#include "nano_network.h"

#define NANOPARSE_CMD_BUF_LEN 1024

static const char TAG[] = "nano_parse";

typedef struct {
    void *cb;    // user callback
    void *param; // user parameters to feed to the user callback
} cb_struct_t;

#define MALLOC( x ) ({\
    size_t size = x; \
    void *mem; \
    mem = malloc( size ); \
    if( NULL == mem ) { \
        goto exit; \
    } \
    mem; \
})

#define FREE_IF_NOT_NULL(x) \
    if( NULL != x ) { \
        free( x ); \
        x = NULL; \
    }

#define JOLT_ERR_CHECK(x) \
    if( E_SUCCESS != x ) { \
        goto exit; \
    }

#define CMD_PREAMBLE \
    char rpc_command[NANOPARSE_CMD_BUF_LEN] = { 0 }; \
    cb_struct_t *s; \
    s = malloc( sizeof(cb_struct_t) ); \
    if( NULL == s ) { \
        return ESP_FAIL; \
    } \
    s->cb = cb; \
    s->param = param;

#define CMD_POSTAMBLE( x ) \
    return jolt_network_post( rpc_command, x, s );
    

/* Last part of any internal cb. Free the response, call the cb, then return */
#define CALL_CB( ... ) \
    FREE_IF_NOT_NULL( response ); \
    if( NULL != cb ) { \
        cb(__VA_ARGS__, p); \
    } \
    return;

/* Put variable declarations before the preamble */
#define CB_PREAMBLE(cb_type) \
    cb_struct_t *s = (cb_struct_t *)param; \
    cb_type cb = (cb_type)(s->cb); \
    void *p = s->param; \
    free( s ); \
    if( 200 != status_code ) { \
        goto exit; \
    }

static void nano_network_block_count_cb(int16_t status_code, char *response, void *param) {
    uint32_t block_count = 0;
    CB_PREAMBLE( nano_network_block_count_cb_t );
    printf("%s\n", response);
    block_count = nanoparse_block_count(response);
    CALL_CB( block_count );
exit:
    CALL_CB( 0 );
}
esp_err_t nano_network_block_count(nano_network_block_count_cb_t cb, void *param) {
    CMD_PREAMBLE;
    snprintf( rpc_command, sizeof(rpc_command),
            "{\"action\":\"block_count\"}" );
    CMD_POSTAMBLE(nano_network_block_count_cb);
}


static void nano_network_work_cb(int16_t status_code, char *response, void *param) {
    uint64_t work = 0;
    CB_PREAMBLE( nano_network_work_cb_t );
    nanoparse_work(response, &work);
    CALL_CB( work );
exit:
    CALL_CB( 0 ); // technically 0 is a valid value, but there's no real consequence.
}
esp_err_t nano_network_work( const hex256_t hash, nano_network_work_cb_t cb, void *param ){
    CMD_PREAMBLE;
    hex256_t hash_upper;
    strlcpy( hash_upper, hash, sizeof(hash_upper) );
    strupr( hash_upper );
    snprintf( (char *) rpc_command, sizeof(rpc_command),
            "{\"action\":\"work_generate\",\"hash\":\"%s\"}",
            hash_upper );
    CMD_POSTAMBLE( nano_network_work_cb );
}


static void nano_network_account_frontier_cb(int16_t status_code, char *response, void *param) {
    char *frontier_block_hash = NULL;
    CB_PREAMBLE( nano_network_account_frontier_cb_t );
    frontier_block_hash = MALLOC( sizeof(hex256_t) );
    JOLT_ERR_CHECK( nanoparse_account_frontier(response, frontier_block_hash) );
    CALL_CB( frontier_block_hash );
exit:
    FREE_IF_NOT_NULL( frontier_block_hash );
    CALL_CB( NULL );
}
esp_err_t nano_network_account_frontier( const char *account_address, nano_network_account_frontier_cb_t cb, void *param ){
    CMD_PREAMBLE;
    snprintf( (char *) rpc_command, sizeof(rpc_command),
            "{\"action\":\"accounts_frontiers\",\"accounts\":[\"%s\"]}",
            account_address);
    CMD_POSTAMBLE( nano_network_account_frontier_cb );
}


static void nano_network_block_cb(int16_t status_code, char *response, void *param) {
    nl_block_t *block = NULL;
    CB_PREAMBLE( nano_network_block_cb_t );
    block = MALLOC( sizeof(nl_block_t) );
    nl_block_init( block );
    JOLT_ERR_CHECK( nanoparse_block(response, block) );
    CALL_CB( block );
exit:
    FREE_IF_NOT_NULL( block );
    CALL_CB( block );
}
esp_err_t nano_network_block( const hex256_t block_hash, nano_network_block_cb_t cb, void *param ){
    CMD_PREAMBLE;
    snprintf( (char *) rpc_command, sizeof(rpc_command),
             "{\"action\":\"block\",\"hash\":\"%s\"}",
             block_hash);
    CMD_POSTAMBLE( nano_network_block_cb );
}


static void nano_network_pending_hash_cb(int16_t status_code, char *response, void *param) {
    char *pending_block_hash = NULL;
    mbedtls_mpi *amount = NULL;
    CB_PREAMBLE( nano_network_pending_hash_cb_t );

    pending_block_hash = MALLOC( sizeof(hex256_t) );

    amount = MALLOC( sizeof(mbedtls_mpi) );
    mbedtls_mpi_init(amount);

    JOLT_ERR_CHECK( nanoparse_pending_hash(response, pending_block_hash, amount) );

    CALL_CB( pending_block_hash, amount );
exit:
    FREE_IF_NOT_NULL( pending_block_hash );
    FREE_IF_NOT_NULL( amount );
    CALL_CB( NULL, NULL );
}
esp_err_t nano_network_pending_hash( const char *account_address, nano_network_pending_hash_cb_t cb, void *param ){
    CMD_PREAMBLE;
    snprintf( (char *) rpc_command, sizeof(rpc_command),
             "{\"action\":\"accounts_pending\","
             "\"count\": 1,"
             "\"source\": \"true\","
             "\"accounts\":[\"%s\"]}",
             account_address);
    CMD_POSTAMBLE( nano_network_pending_hash_cb );
}


static void nano_network_frontier_block_cb(hex256_t frontier_block_hash, void *param) {
    /* Once the block comes back, directly call the user callback, providing 
     * user parameters */
    cb_struct_t *s = (cb_struct_t *)param;
    nano_network_frontier_block_cb_t cb = (nano_network_frontier_block_cb_t)(s->cb);
    void *p = s->param;
    free( s );
    nano_network_block( frontier_block_hash, cb, p );
    FREE_IF_NOT_NULL( frontier_block_hash );
}
esp_err_t nano_network_frontier_block( const char *address, nano_network_frontier_block_cb_t cb, void *param ){
    /* Convenience function to get frontier hash and block contents.
     * Fills in block's field according to account in block->account.
     *
     * General Steps:
     *     1) Get Frontier Block Hash
     *     2) Query that hash
     *     3) Call the User cb
     */

    /* 1) Get Frontier Block Hash */
    cb_struct_t *s;
    s = malloc( sizeof(cb_struct_t) );
    if( NULL == s ) {
        return ESP_FAIL;
    }
    s->cb = cb;
    s->param = param;

    return nano_network_account_frontier( address,
            nano_network_frontier_block_cb, s );
}

static void nano_network_process_cb(int16_t status_code, char *response, void *param) {
    CB_PREAMBLE( nano_network_process_cb_t );
    /* Todo: check response data */
exit:
    CALL_CB( status_code );
}
esp_err_t nano_network_process( const nl_block_t *block, nano_network_process_cb_t cb, void *param) {
    CMD_PREAMBLE;
    nanoparse_process(block, rpc_command, sizeof(rpc_command));
    CMD_POSTAMBLE( nano_network_process_cb );
}

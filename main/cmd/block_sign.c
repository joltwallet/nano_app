#include "confirmation.h"
#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"
#include "nano_parse.h"

static const char TAG[] = "nano_sign_block";

typedef struct {
    int response;
    uint32_t account_index;
    struct {
        nl_block_t frontier;
        nl_block_t new;
    } block;
} sign_block_obj_t;

static void cleanup_complete( void *param );
static void vault_failure_cb( void *param );

static void step_1( void *param );
static void step_2( bool confirm, void *param );
static void step_3( void *param );

/**
 * @brief cleanup the entire send context.
 *
 * Called upon exit, completion, OOM-errors, or corrupt data.
 */
static void cleanup_complete( void *param )
{
    sign_block_obj_t *d = param;
    jolt_cli_return( d->response );
    free( d );
}

static void vault_failure_cb( void *param ) { jolt_cli_return( 0 ); }

/**
 * @brief Validate inputs, create confirmation prompt screen
 */
static void step_1( void *param )
{
    char **argv         = param;
    sign_block_obj_t *d = NULL;

    d = malloc( sizeof( sign_block_obj_t ) );
    if( NULL == d ) {
        ESP_LOGE( TAG, "Could not allocate memory for sign_obj_t" );
        goto exit;
    }
    nl_block_init( &d->block.frontier );
    nl_block_init( &d->block.new );
    d->response = -1;

    /* Parse Blocks Into Json */
    if( E_SUCCESS != nanoparse_block( argv[2], &d->block.frontier ) ) {
        printf( "Unable to parse frontier block\n" );
        d->response = 2;
        goto exit;
    }
    if( E_SUCCESS != nanoparse_block( argv[3], &d->block.new ) ) {
        printf( "Unable to parse block to be signed\n" );
        d->response = 3;
        goto exit;
    }

    d->account_index = atoi( argv[1] );
    if( 0 == d->account_index && '0' != argv[1][0] ) {
        printf( "Invalid Index" );
        d->response = 4;
        goto exit;
    }

    nano_confirm_block( &d->block.frontier, &d->block.new, step_2, d );

    return;

exit:
    if( NULL == d ) { jolt_cli_return( -1 ); }
    else {
        cleanup_complete( d );
    }
}

/**
 * @brief Refresh vault in preparation to sign
 */
static void step_2( bool confirm, void *param )
{
    sign_block_obj_t *d = param;

    if( !confirm ) {
        cleanup_complete( d );
        return;
    }

    vault_refresh( cleanup_complete, step_3, d );
}

/**
 * @brief Sign, format, and print
 */
static void step_3( void *param )
{
    sign_block_obj_t *d = param;

    /* Sign Send Block */
    ESP_LOGI( TAG, "Signing Block" );
    {
        CONFIDENTIAL uint256_t private_key;
        jolt_err_t res;

        if( !nano_index_get_private( private_key, d->account_index ) ) {
            ESP_LOGI( TAG, "Error getting private key" );
            goto exit;
        }
        res = nl_block_sign( &d->block.new, private_key );
        sodium_memzero( private_key, sizeof( private_key ) );
        if( E_SUCCESS != res ) {
            ESP_LOGI( TAG, "Error Signing Block" );
            goto exit;
        }
    }

    /* Parse Signed Block Back Into Json */
    char buf[1024];
    if( E_SUCCESS != nanoparse_process( &d->block.new, buf, sizeof( buf ) ) ) {
        printf( "Error Parsing Signed Block\n" );
        d->response = 6;
        goto exit;
    }

    /* Print signed block back */
    /* todo: maybe just print signature */
    printf( buf );
    printf( "\n" );

    d->response = 0;

    /* fall through */

exit:
    cleanup_complete( d );
}

int nano_cmd_block_sign( int argc, char **argv )
{
    if( !console_check_equal_argc( argc, 4 ) ) { return -1; }

    vault_refresh( vault_failure_cb, step_1, argv );

    return JOLT_CLI_NON_BLOCKING;
}

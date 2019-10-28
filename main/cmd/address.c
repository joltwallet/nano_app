#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"

static uint32_t lower, upper;

void success_cb( void *param )
{
    /* Not using cJSON because it's simpler to do it manually in a memory-friendly
     * way */
    JOLT_NO_LOGGING_CTX
    {
        printf( "{\"addresses\":[" );
        for( uint32_t index = lower; index <= upper; index++ ) {
            char address[ADDRESS_BUF_LEN] = {0};
            nano_index_get_address( address, index );
            printf( "{\"index\":%d,\"address\":\"%s\"}", index, address );
            if( index != upper ) printf( "," );
        }
        printf( "]}" );
    }

    jolt_cli_return( 0 );
}

void failure_cb( void *param ) { jolt_cli_return( -1 ); }

int nano_cmd_address( int argc, char **argv )
{
    /* Argument Verification */
    if( !console_check_range_argc( argc, 1, 3 ) ) { return 1; }

    /* Convert arguments to ints.
     * Note: atoi returns 0 if provided argument cannot be converted to integer.
     */
    if( 1 == argc ) {
        /* Print only currently selected address */
        lower = nano_index_get( NULL );
        upper = lower;
    }
    else {
        lower = atoi( argv[1] );
        if( 3 == argc ) { upper = atoi( argv[2] ); }
        else {
            upper = lower;
        }
    }

    vault_refresh( failure_cb, success_cb, NULL );

    return JOLT_CLI_NON_BLOCKING;
}

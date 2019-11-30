//#define LOG_LOCAL_LEVEL 4

#include "esp_log.h"
#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"
#include "errno.h"

static uint32_t lower=0, upper=0;
static const char TAG[] = "nano_cmd_address";

enum {
    NANO_ADDRESS_INVALID_ARGC = -1,
    NANO_ADDRESS_INVALID_LWR = -2,
    NANO_ADDRESS_INVALID_UPR = -3,
    NANO_ADDRESS_INVALID_RANGE = -4,
};

void success_cb( void *param )
{
    /* Not using cJSON because it's simpler to do it manually in a memory-friendly
     * way */
    JOLT_NO_LOGGING_CTX
    {
        printf( "{\"addresses\":[" );
        for( uint32_t index = lower; index <= upper; index++ ) {
            char address[ADDRESS_BUF_LEN] = {0};
            if( !nano_index_get_address( address, index ) ) abort();
            printf( "{\"index\":%u,\"address\":\"%s\"}", index, address );
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
    if( !console_check_range_argc( argc, 1, 3 ) ) { return NANO_ADDRESS_INVALID_ARGC; }

    /* Convert arguments to ints.
     * Note: atoi returns 0 if provided argument cannot be converted to integer.
     */
    if( 1 == argc ) {
        /* Print only currently selected address */
        lower = nano_index_get( NULL );
        upper = lower;
        if( lower > INT32_MAX ) return NANO_ADDRESS_INVALID_LWR; 
    }
    else {
        char *endptr;

        errno = 0;
        lower = strtoul( argv[1], &endptr, 10 );
        if(lower > INT32_MAX || argv[1] == endptr || 0 != errno ) return NANO_ADDRESS_INVALID_LWR;

        if( 3 == argc ) { 
            errno = 0;
            upper = strtoul( argv[2], &endptr, 10 );
            if( upper > INT32_MAX || argv[2] == endptr || 0 != errno ) return NANO_ADDRESS_INVALID_UPR;
            if( upper < lower ) return NANO_ADDRESS_INVALID_RANGE;
        }
        else {
            upper = lower;
        }
    }
    ESP_LOGD(TAG, "Lower: %u, Upper: %u", lower, upper);

    vault_refresh( failure_cb, success_cb, NULL );

    return JOLT_CLI_NON_BLOCKING;
}

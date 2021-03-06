#include "jolt_lib.h"
#include "nano_network.h"

static void network_cb( uint32_t count, void *param, jolt_gui_obj_t *scr )
{
    /* Create the text screen */
    if( count > 0 ) {
        printf( "%d\n", count );
        jolt_cli_return( 0 );
    }
    else {
        jolt_cli_return( -1 );
    }
}

int nano_cmd_count( int argc, char **argv )
{
    nano_network_block_count( network_cb, NULL, NULL );
    return JOLT_CLI_NON_BLOCKING;
}

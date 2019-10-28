#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"

static const char TAG[]   = "nano_contact";
static const char TITLE[] = "Contacts";

static const char confirmation_add_str[]    = "Add contact:\nName: %s\nAddress:";
static const char confirmation_update_str[] = "Update contact %d to:\nName: %s\nAddress:";

static cJSON *json     = NULL;
static cJSON *contacts = NULL;
static int idx         = 0;
static char *name      = NULL;
static char *address   = NULL;

static void cleanup( int return_code )
{
    jolt_json_del( json );
    idx      = 0;
    json     = NULL;
    contacts = NULL;
    free( name );
    free( address );
    jolt_cli_return( return_code );
}

static void confirmation_cb( jolt_gui_obj_t *obj, jolt_gui_event_t event )
{
    if( jolt_gui_event.short_clicked == event ) {
        cJSON *new_contact = cJSON_CreateObject();
        if( NULL == cJSON_AddStringToObject( new_contact, "name", name ) ) {
            ESP_LOGE( TAG, "Failed to add string object \"name\"" );
        }
        if( NULL == cJSON_AddStringToObject( new_contact, "address", address ) ) {
            ESP_LOGE( TAG, "Failed to add string object \"address\"" );
        }
        cJSON_AddItemToArray( contacts, new_contact );
        jolt_json_write_app( json );
        jolt_gui_scr_del( obj );
        cleanup( 0 );
    }
    else if( jolt_gui_event.cancel == event ) {
        jolt_gui_scr_del( obj );
        cleanup( -1 );
    }
}

static int confirmation_create()
{
    char buf[strlen( confirmation_update_str ) + CONFIG_JOLT_NANO_CONTACTS_NAME_LEN + 1];
    /* Verify it's a valid address */
    {
        uint256_t pub_key;
        if( E_SUCCESS != nl_address_to_public( pub_key, address ) ) {
            printf( "Invalid address: %s\n", address );
            return 1;
        }
        /* Copy the name and address to persist */
        char *tmp = NULL;
        tmp       = malloc( strlen( address ) + 1 );
        strcpy( tmp, address );
        address = tmp;

        tmp = malloc( strlen( name ) + 1 );
        strcpy( tmp, name );
        name = tmp;
    }
    if( idx < 0 ) { snprintf( buf, sizeof( buf ), confirmation_add_str, name ); }
    else {
        snprintf( buf, sizeof( buf ), confirmation_update_str, idx + 1, name );
    }
    jolt_gui_obj_t *scr = NULL;
    scr                 = jolt_gui_scr_text_create( TITLE, buf );
    jolt_gui_scr_scroll_add_monospace_text( scr, address );
    jolt_gui_scr_set_event_cb( scr, confirmation_cb );
    return JOLT_CLI_NON_BLOCKING;
}

int nano_cmd_contact( int argc, char **argv )
{
    /* Argument Verification */
    if( !console_check_range_argc( argc, 2, 5 ) ) { return 1; }

    json           = nano_get_json();
    contacts       = cJSON_GetObjectItemCaseSensitive( json, "contacts" );
    int n_contacts = cJSON_GetArraySize( contacts );

    /* More specific argument verification */
    if( 0 == strcmp( argv[1], "print" ) ) { printf( "not yet implemented\n" ); }
    else if( 0 == strcmp( argv[1], "delete" ) ) {
        printf( "not yet implemented\n" );
    }
    else if( 0 == strcmp( argv[1], "add" ) ) {
        if( !console_check_equal_argc( argc, 4 ) ) { return 1; }
        if( n_contacts >= CONFIG_JOLT_NANO_CONTACTS_MAX ) { return 1; }
        idx     = -1;
        name    = argv[2];
        address = argv[3];

        if( strlen( name ) > CONFIG_JOLT_NANO_CONTACTS_NAME_LEN ) {
            printf( "Too long name.\n" );
            return 1;
        }

        return confirmation_create();
    }
    else if( 0 == strcmp( argv[1], "insert" ) ) {
        if( n_contacts >= CONFIG_JOLT_NANO_CONTACTS_MAX ) { return 1; }
        printf( "not yet implemented\n" );
    }
    else if( 0 == strcmp( argv[1], "update" ) ) {
        printf( "not yet implemented\n" );
    }
    else if( 0 == strcmp( argv[1], "clear" ) ) {
        printf( "not yet implemented\n" );
    }
    return -1;
}

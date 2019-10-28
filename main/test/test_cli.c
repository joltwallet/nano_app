#include "japp.h"
#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"
#include "unity.h"

static const char MODULE_NAME[] = "[nano_app/cli]";

#define argcount( x ) ( sizeof( x ) / sizeof( const char * ) )

TEST_CASE( "address", MODULE_NAME )
{
    vault_set_unit_test( CONFIG_APP_COIN_PATH, CONFIG_APP_BIP32_KEY );
    int res;

    /* No Index */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":0,\"address\":\"nano_"
                                  "3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\"}]}",
                                  buf );
    }

    /* Single Index */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "0"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":0,\"address\":\"nano_"
                                  "3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\"}]}",
                                  buf );
    }

    /* Range */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "2", "5"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":2,\"address\":\"nano_"
                                  "38j3a6enhn4af9pibzzfo69x89qu1n6ogi4edharx5uqh134k5g9n35ha997\"},{"
                                  "\"index\":3,\"address\":\"nano_"
                                  "3ktj8umiumx1n6otpi7fkmfi5ci1irqofushfwetsdqdnohxbbap6hn6o7n5\"},{"
                                  "\"index\":4,\"address\":\"nano_"
                                  "1gi536w1r1fe4q8i9qdmpenp4w84r6d8mpefyezxg77mywobmzcj7ka1umzw\"},{"
                                  "\"index\":5,\"address\":\"nano_"
                                  "1wijizqdobawi6596botmgwaefr38dg643nf7ky93d9fs79qx5knys57foxa\"}]}",
                                  buf );
    }
}

TEST_CASE( "contact", MODULE_NAME )
{
    vault_set_unit_test( CONFIG_APP_COIN_PATH, CONFIG_APP_BIP32_KEY );
    printf( "test" );
    TEST_FAIL();
}

TEST_CASE( "block_sign", MODULE_NAME )
{
    vault_set_unit_test( CONFIG_APP_COIN_PATH, CONFIG_APP_BIP32_KEY );
    printf( "test" );
    TEST_FAIL();
}

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
    jolt_json_del_app();

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
    jolt_json_del_app();

    /* Add */
    JOLT_CLI_UNIT_TEST_CTX(4096)
    {
        const char *argv[] = {"contact", "add", "Satoshi Nakamoto", "xrb_1jo1twa11et111111111111111111111111111111111111111118n3ca6q5"};
        TEST_ASSERT_EQUAL_INT_MESSAGE( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ), buf );
        vTaskDelay(pdMS_TO_TICKS(50));
        JOLT_ENTER;  /* Accept new contact */
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"name\":\"Satoshi Nakamoto\","
                "\"address\":\"xrb_1jo1twa11et111111111111111111111111111111111111111118n3ca6q5\","
                "\"index\":1}",
                buf);
    }
}

TEST_CASE( "block_sign", MODULE_NAME )
{
    vault_set_unit_test( CONFIG_APP_COIN_PATH, CONFIG_APP_BIP32_KEY );
    jolt_json_del_app();

    /* Successful Send */
    //JOLT_CLI_UNIT_TEST_CTX(4096)
    {
        const char *argv[] = {
            "block_sign",
            "0",
            "{\"type\": \"state\","
                "\"account\": \"nano_3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\","
                "\"previous\": \"DEADBEEF000102030405060708090A0B0C0D0E0F101112131415161718191A1B\","
                "\"representative\": \"nano_1cwswatjifmjnmtu5toepkwca64m7qtuukizyjxsghujtpdr9466wjmn89d8\","
                "\"balance\": \"200000000000000000000000000000000000\","
                "\"link\": \"095B645B6C0CCCB52DD65218DE613CE13CEA58A850A80C3F704291B698A50417\"}",
            "{\"type\": \"state\","
                "\"account\": \"nano_3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\","
                "\"previous\": \"56D2E7D412223C9E7432C2F80D21996CA670CD20798C1E67F6AE3847D947E0BE\","
                "\"representative\": \"nano_1cwswatjifmjnmtu5toepkwca64m7qtuukizyjxsghujtpdr9466wjmn89d8\","
                "\"balance\": \"100000000000000000000000000000000000\","
                "\"link\": \"42DD308BA91AA225B9DD0EF15A68A8DD49E2940C6277A4BFAC363E1C8BF14279\"}",
        };
        japp_main( argcount( argv ), argv );
        //TEST_ASSERT_EQUAL_INT_MESSAGE( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ), buf );
        vTaskDelay(pdMS_TO_TICKS(500));
        JOLT_ENTER;  /* Accept signing prompt */
        vTaskDelay(pdMS_TO_TICKS(50));
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        //TEST_ASSERT_EQUAL_STRING( "", buf);
    }

    TEST_FAIL();
}

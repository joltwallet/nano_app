#include "japp.h"
#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"
#include "unity.h"

static const char MODULE_NAME[] = "[nano_app/cli]";

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
                                  "3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\"}]}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Single Index (0) */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "0"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":0,\"address\":\"nano_"
                                  "3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\"}]}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Single Index (1) */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "1"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":1,\"address\":\"nano_"
                                  "1u8yk67t9g3bhpym66yef3g89cef5fx6y5ttas7e9ej37k6edu9zew43wzsp\"}]}"
                                  "{\"exit_code\":0}",
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
                                  "1wijizqdobawi6596botmgwaefr38dg643nf7ky93d9fs79qx5knys57foxa\"}]}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Single Index (Large <INT32_MAX Index) */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "1000000000"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":1000000000,\"address\":\"nano_"
                                  "14mfj9wmhbnhoc538rexbpzg9nhj84wphhmariy4sfurknzbs5xiibuibmon\"}]}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Single Index (Max Index) */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "2147483647"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":2147483647,\"address\":\"nano_"
                                  "3eckfoihizwk4axo5a7z4tj31bchuer48ysnderzwrdoteapee8ygsw6dtmr\"}]}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Single Index (>Max Index) */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "2147483648"};
        TEST_ASSERT_EQUAL_INT( -2, japp_main( argcount( argv ), argv ) );
    }

    /* Invalid Index (Negative) */
    {
        const char *argv[] = {"address", "-1"};
        TEST_ASSERT_EQUAL_INT( -2, japp_main( argcount( argv ), argv ) );
    }

    /* Max Index Upper */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "2147483647", "2147483647"};
        TEST_ASSERT_EQUAL_INT( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"addresses\":[{\"index\":2147483647,\"address\":\"nano_"
                                  "3eckfoihizwk4axo5a7z4tj31bchuer48ysnderzwrdoteapee8ygsw6dtmr\"}]}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* >Max Index Upper */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "2147483647", "21474836478"};
        TEST_ASSERT_EQUAL_INT( -3, japp_main( argcount( argv ), argv ) );
    }

    /* Invalid Range (Upper < Lower) */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {"address", "4", "3"};
        TEST_ASSERT_EQUAL_INT( -4, japp_main( argcount( argv ), argv ) );
    }
}

TEST_CASE( "block_sign", MODULE_NAME )
{
    vault_set_unit_test( CONFIG_APP_COIN_PATH, CONFIG_APP_BIP32_KEY );
    jolt_json_del_app();

    /* Successful Send */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {
                "block_sign", "0",
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
                "\"link\": \"42DD308BA91AA225B9DD0EF15A68A8DD49E2940C6277A4BFAC363E1C8BF14279\"}"};
        TEST_ASSERT_EQUAL_INT_MESSAGE( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ), buf );
        JOLT_ENTER; /* Accept signing prompt */
        vTaskDelay( pdMS_TO_TICKS( 50 ) );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"signature\":"
                                  "\"6AE1CB46AFB31C682EDD04286D320BB295108912C52AD801194E0AE9E607D88A9222545DA78A3C328"
                                  "D01EFEB42074807FE79562CF4D2E3213606D77E6D128D09\"}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Successful recieve */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {
                "block_sign", "0",
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
                "\"balance\": \"300000000000000000000000000000000000\","
                "\"link\": \"42DD308BA91AA225B9DD0EF15A68A8DD49E2940C6277A4BFAC363E1C8BF14279\"}"};
        TEST_ASSERT_EQUAL_INT_MESSAGE( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ), buf );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
    }

    /* Successfull OPEN block */
    JOLT_CLI_UNIT_TEST_CTX( 4096 )
    {
        const char *argv[] = {
                "block_sign", "0",
                "{\"type\": \"state\","
                "\"account\": \"nano_3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz\","
                "\"previous\": \"0000000000000000000000000000000000000000000000000000000000000000\","
                "\"representative\": \"nano_1cwswatjifmjnmtu5toepkwca64m7qtuukizyjxsghujtpdr9466wjmn89d8\","
                "\"balance\": \"200000000000000000000000000000000000\","
                "\"link\": \"095B645B6C0CCCB52DD65218DE613CE13CEA58A850A80C3F704291B698A50417\"}"};
        // Even though this should require no confirmation, the CLI returns
        // JOLT_CLI_NON_BLOCKING because the vault may need a PIN entry.
        TEST_ASSERT_EQUAL_INT_MESSAGE( JOLT_CLI_NON_BLOCKING, japp_main( argcount( argv ), argv ), buf );
        TEST_ASSERT_EQUAL_INT( 0, jolt_cli_get_return() );
        TEST_ASSERT_EQUAL_STRING( "{\"signature\":"
                                  "\"9C6FA0D277C6D294909E437AA13AD13D96491FF05E664D70F8CB0352B267F0AA70C4D9CA74AA7C208"
                                  "0BA7F23D320C33CBCEFF549E3CA637690A717C01521B80B\"}"
                                  "{\"exit_code\":0}",
                                  buf );
    }

    /* Only a rep change */
    // TODO

    /* Rep change and a send */
    // TODO

    /* Rep change and a receive */
    // TODO

    /* Invalid "previous" hash */
    // TODO

    /* Large account index */
    // TODO

    /* Invalidly large account index */
    // TODO

    /* Invalid Negative account index */
    // TODO
}

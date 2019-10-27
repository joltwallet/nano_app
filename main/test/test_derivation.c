#include "jolt_lib.h"
#include "nano_helpers.h"
#include "nano_lib.h"
#include "unity.h"

static const char MODULE_NAME[] = "[nano_app/derivation]";

TEST_CASE( "address derivation", MODULE_NAME )
{
    vault_set_unit_test(CONFIG_APP_COIN_PATH, CONFIG_APP_BIP32_KEY);

    char address[ADDRESS_BUF_LEN] = { 0 };
    nano_index_get_address(address, 0);
    TEST_ASSERT_EQUAL_STRING(
            "nano_3of1t4mf4y8udapj45zgg5bewmc79sagbmwifsmaikbmyzodst1hk55pjqcz",
            address);

    nano_index_get_address(address, 1);
    TEST_ASSERT_EQUAL_STRING(
            "nano_1u8yk67t9g3bhpym66yef3g89cef5fx6y5ttas7e9ej37k6edu9zew43wzsp",
            address);
}


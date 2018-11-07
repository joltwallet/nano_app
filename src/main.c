#include <stddef.h>
#include "sodium.h"

#include "jolt_gui/jolt_gui.h"
#include "menus/submenus.h"
//#include "nano_console.h"

int app_main(int argc, char **argv) {
    const char title[] = "Nano";
    lv_obj_t *menu = jolt_gui_scr_menu_create(title);
    jolt_gui_scr_menu_add(menu, NULL, "Balance", menu_nano_balance);
    jolt_gui_scr_menu_add(menu, NULL, "Receive", NULL);
    jolt_gui_scr_menu_add(menu, NULL, "Send (contact)", NULL);
    jolt_gui_scr_menu_add(menu, NULL, "Block Count", menu_nano_block_count);
    jolt_gui_scr_menu_add(menu, NULL, "Select Account", menu_nano_select_account);
    jolt_gui_scr_menu_add(menu, NULL, "Address (text)", menu_nano_address_text);
    jolt_gui_scr_menu_add(menu, NULL, "Address (QR)", menu_nano_address_qr);

    /* Always return the pointer to the main app menu */
    return menu;
}

#if 0
int console(int argc, char **argv);

int app_main(int argc, char **argv) {
    /* Entry point for primary Jolt GUI */
    menu8g2_t nano_menu;
    menu8g2_copy(&nano_menu, menu);

    const char title[] = "Nano";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 7);
    menu8g2_set_element(&elements, "Balance", &menu_nano_balance);
    menu8g2_set_element(&elements, "Receive", &menu_nano_receive);
    menu8g2_set_element(&elements, "Send (contact)", &menu_nano_send_contact);
    menu8g2_set_element(&elements, "Block Count", &menu_nano_block_count);
    menu8g2_set_element(&elements, "Select Account", &menu_nano_select_account);
    menu8g2_set_element(&elements, "Address (text)", &menu_nano_address_text);
    menu8g2_set_element(&elements, "Address (QR)", &menu_nano_address_qr);
    menu8g2_create_vertical_element_menu(&nano_menu, title, &elements);
    menu8g2_elements_free(&elements);

    return 0;
}

#if CONFIG_JOLT_NANO_CONSOLE_ENABLE
#include "console.h"
#if 0
static int meow(int argc, char **argv) {
    printf("meowmeowmeowmeow\n");
    return 0;
}
static int doge(int argc, char **argv) {
    printf("dogedogedoge\n");
    return 0;
}
#endif
int console(int argc, char **argv) {
    /* Entry point for console commands */
    esp_console_cmd_t cmd;
    subconsole_t *subconsole = subconsole_cmd_init();
    console_nano_register(subconsole);
#if 0
    cmd = (esp_console_cmd_t) {
        .command = "meow",
        .help = "Get the current Nano block count",
        .func = &meow,
    };
    subconsole_cmd_register(subconsole, &cmd);

    cmd = (esp_console_cmd_t) {
        .command = "doge",
        .help = "Such wow",
        .func = &doge,
    };
    subconsole_cmd_register(subconsole, &cmd);
#endif

    subconsole_cmd_run(subconsole, argc, argv);
    subconsole_cmd_free(subconsole);

#if 0
    cmd = (esp_console_cmd_t) {
        .command = "count",
        .help = "Get the current Nano block count",
        .func = &nano_count,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "balance",
        .help = "Get the current Nano Balance",
        .hint = NULL,
        .func = &nano_balance,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "address",
        .help = "Get the Nano Address at derivation index or index range",
        .hint = NULL,
        .func = &nano_address,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "sign_block",
        .help = "Given the index, head block, and the block to be signed,"
                "prompt user to sign",
        .hint = NULL,
        .func = &nano_sign_block,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "send",
        .help = "WiFi send. Inputs: account index, dest address, amount (raw)",
        .hint = NULL,
        .func = &nano_send,
    };
    ESP_ERROR_CHECK( esp_console_cmd_register(&cmd) );

    cmd = (esp_console_cmd_t) {
        .command = "contact_update",
        .help = "Update Nano Contact (index, name, address)",
        .hint = NULL,
        .func = &nano_contact_update,
    };




    cmd = (esp_console_cmd_t) {
        .command = "free",
        .help = "Get the total size of heap memory available",
        .hint = NULL,
        .func = &free_mem,
    };
    if( argc == 0 ) {
        printf("Nano Console. Available Functions are:")
    }
    printf("First Passed in argument was %s.\n", argv[0]);
#endif
    return 0;
}
#endif
#endif

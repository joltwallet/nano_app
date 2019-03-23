#include <stddef.h>
#include "jolt_lib.h"
//#include "sodium.h"

//#include "jolt_gui/jolt_gui.h"
#include "menus/submenus.h"
//#include "nano_console.h"
#include "cmd/cmds.h"

static const char TAG[] = "nano_main";
static int console(int argc, char **argv);

int app_main(int argc, char **argv) {
    ESP_LOGI(TAG, "argc: %d", argc);
    lv_obj_t *menu = NULL;
    if( 0 == argc)  {
        const char title[] = "Nano";
        menu = jolt_gui_scr_menu_create(title);
        jolt_gui_scr_menu_add(menu, NULL, "Balance", menu_nano_balance);
        jolt_gui_scr_menu_add(menu, NULL, "Receive", menu_nano_receive);
        jolt_gui_scr_menu_add(menu, NULL, "Send (contact)", NULL);
        jolt_gui_scr_menu_add(menu, NULL, "Block Count", menu_nano_block_count);
        jolt_gui_scr_menu_add(menu, NULL, "Select Account", menu_nano_select_account);
        jolt_gui_scr_menu_add(menu, NULL, "Address", menu_nano_address);
        jolt_gui_scr_menu_add(menu, NULL, "About", NULL);
        return (int)menu;
    }
    else {
        return console(argc, argv);
    }
}


static int meow(int argc, char **argv) {
    printf("meowmeowmeowmeow\n");
    return 0;
}
static int doge(int argc, char **argv) {
    printf("dogedogedoge\n");
    return 0;
}

static int console(int argc, char **argv) {
    /* Entry point for console commands */
    esp_console_cmd_t cmd;
    subconsole_t *subconsole = subconsole_cmd_init();
    //console_nano_register(subconsole);
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

    /* Commands shouldn't rely on WiFi networking; this command is just an example */
    cmd = (esp_console_cmd_t) {
        .command = "count",
        .help = "Get the current Nano block count",
        .func = &nano_count,
    };
    subconsole_cmd_register(subconsole, &cmd);

    cmd = (esp_console_cmd_t) {
        .command = "address",
        .help = "Get the Nano Address at derivation index or index range",
        .hint = NULL,
        .func = &nano_address,
    };
    subconsole_cmd_register(subconsole, &cmd);

#if 0
    cmd = (esp_console_cmd_t) {
        .command = "balance",
        .help = "Get the current Nano Balance",
        .hint = NULL,
        .func = &nano_balance,
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

    ESP_LOGD(TAG, "Running %s", argv[0]);
    int res = subconsole_cmd_run(subconsole, argc, argv);

    ESP_LOGD(TAG, "Freeing subconsole");
    subconsole_cmd_free(subconsole);

    ESP_LOGD(TAG, "App exiting with code %d", res);
    return res;
}

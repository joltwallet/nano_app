#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <stddef.h>
#include "jolt_lib.h"

#include "menus/submenus.h"
#include "cmd/cmds.h"

static const char TAG[] = "nano_main";
static int console(int argc, char **argv);

int japp_main(int argc, char **argv) {
    ESP_LOGI(TAG, "argc: %d", argc);
    jolt_gui_obj_t *menu = NULL;
    if( 0 == argc)  {
        const char title[] = "Nano";
        menu = jolt_gui_scr_menu_create(title);
        jolt_gui_scr_menu_add(menu, NULL, "Balance", menu_nano_balance);
        jolt_gui_scr_menu_add(menu, NULL, "Receive", menu_nano_receive);
        jolt_gui_scr_menu_add(menu, NULL, "Send (contact)", menu_nano_contacts);
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

static int console(int argc, char **argv) {
    /* Entry point for console commands */
    esp_console_cmd_t cmd;
    jolt_cli_sub_t *subconsole = jolt_cli_sub_init();

    /* Commands shouldn't rely on WiFi networking; this command is just an example */
    cmd = (esp_console_cmd_t) {
        .command = "count",
        .help = "Get the current Nano block count",
        .func = &nano_cmd_count,
    };
    jolt_cli_sub_cmd_register(subconsole, &cmd);

    cmd = (esp_console_cmd_t) {
        .command = "address",
        .help = "Get the Nano Address at derivation index or index range",
        .hint = NULL,
        .func = &nano_cmd_address,
    };
    jolt_cli_sub_cmd_register(subconsole, &cmd);

    cmd = (esp_console_cmd_t) {
        .command = "contact",
        .help = "Update Nano Contact (index, name, address)",
        .hint = NULL,
        .func = &nano_cmd_contact,
    };
    jolt_cli_sub_cmd_register(subconsole, &cmd);

    cmd = (esp_console_cmd_t) {
        .command = "block_sign",
        .help = "Given the index, head block, and the block to be signed,"
                "prompt user to sign",
        .hint = NULL,
        .func = &nano_cmd_block_sign,
    };
    jolt_cli_sub_cmd_register(subconsole, &cmd);

    ESP_LOGD(TAG, "(argc %d) Running %s", argc, argv[0]);
    int res = jolt_cli_sub_cmd_run(subconsole, argc, argv);

    ESP_LOGD(TAG, "Freeing subconsole");
    jolt_cli_sub_cmd_free(subconsole);

    ESP_LOGD(TAG, "App exiting with code %d", res);
    return res;
}

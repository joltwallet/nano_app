#include <stddef.h>
#include "sodium.h"
#include "globals.h"
#include "gui/gui.h"
#include "gui/graphics.h"
//#include "menus/submenus.h"

volatile char *private_key = NULL; // Pointer to Key in Vault

int console(int argc, char **argv);
//extern menu8g2_t *get_menu();
int app_main(int argc, char **argv) {
    /* Entry point for primary Jolt GUI */
    menu8g2_t nano_menu;
    menu8g2_copy(&nano_menu, menu);

    const char title[] = "Nano";

    menu8g2_elements_t elements;
    menu8g2_elements_init(&elements, 7);
    //menu8g2_set_element(&elements, "Balance", &menu_nano_balance);
    menu8g2_set_element(&elements, "Balance", NULL);
    //menu8g2_set_element(&elements, "Receive", &menu_nano_receive);
    menu8g2_set_element(&elements, "Receive", NULL);
    //menu8g2_set_element(&elements, "Send (contact)", &menu_nano_send_contact);
    menu8g2_set_element(&elements, "Send (contact)", NULL);
    //menu8g2_set_element(&elements, "Block Count", &menu_nano_block_count);
    menu8g2_set_element(&elements, "Block Count", NULL);
    //menu8g2_set_element(&elements, "Select Account", &menu_nano_select_account);
    menu8g2_set_element(&elements, "Select Account", NULL);
    //menu8g2_set_element(&elements, "Address (text)", &menu_nano_address_text);
    menu8g2_set_element(&elements, "Address (text)", NULL);
    //menu8g2_set_element(&elements, "Address (QR)", &menu_nano_address_qr);
    menu8g2_set_element(&elements, "Address (QR)", NULL);
    menu8g2_create_vertical_element_menu(&nano_menu, title, &elements);
    menu8g2_elements_free(&elements);

    return 0;
}

int console(int argc, char **argv) {
    /* Entry point for console commands */
    printf("First Passed in argument was %s.\n", argv[0]);
    return 0;
}

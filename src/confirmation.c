/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

/* IMPORTANT: Expects to be called from OUTSIDE the lvgl event loop. This 
 *            function BLOCKS while waiting for user input.
 */


#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "nano_helpers.h"
#include "confirmation.h"

static const char *TAG = "nano_conf";

#define STR_IMPL_(x) #x      //stringify argument
#define STR(x) STR_IMPL_(x)  //indirection to expand argument macros

#if 0
bool nano_confirm_contact_update(const menu8g2_t *prev_menu, const char *name,
        const uint256_t public, const uint8_t index){
    menu8g2_t menu_obj;
    menu8g2_t *m = &menu_obj;
    menu8g2_copy(m, prev_menu);

    char buf[200];
    snprintf(buf, sizeof(buf), "Update Index: %d ?", index);
    if ( !menu_confirm_action(m, buf) ){
        return false;
    }

    snprintf(buf, sizeof(buf), "Name: %s", name);
    if ( !menu_confirm_action(m, buf) ){
        return false;
    }

    char address[ADDRESS_BUF_LEN];
    if( E_SUCCESS != nl_public_to_address(address, sizeof(address), public) ){
        return false;
    }
    snprintf(buf, sizeof(buf), "Address: %s", address);
    if ( !menu_confirm_action(m, buf) ){
        return false;
    }

    return true;
}
#endif




static SemaphoreHandle_t user_input_complete = NULL;
static bool nano_confirm_action_response = false;

static lv_action_t nano_confirm_action_back_cb( lv_obj_t *btn ) {
    // No gui semaphores needed since callbacks are executed in the lvgl event loop
    jolt_gui_scr_del();
    nano_confirm_action_response = false;
    xSemaphoreGive(user_input_complete);
    return LV_RES_INV;
}

static lv_action_t nano_confirm_action_enter_cb( lv_obj_t *btn ) {
    // No gui semaphores needed since callbacks are executed in the lvgl event loop
    jolt_gui_scr_del();
    nano_confirm_action_response = true;
    xSemaphoreGive(user_input_complete);
    return LV_RES_INV;
}

static bool nano_confirm_action(const char *title, const char *text) {
    /*
     */
    if( NULL == user_input_complete ) {
        user_input_complete = xSemaphoreCreateBinary();
    }

    jolt_gui_sem_take();
    lv_obj_t *scr = jolt_gui_scr_text_create(title, text);
    jolt_gui_scr_set_back_action(scr, nano_confirm_action_back_cb);
    jolt_gui_scr_set_enter_action(scr, nano_confirm_action_enter_cb);
    jolt_gui_sem_give();

	// Block waiting for user response
    xSemaphoreTake( user_input_complete, portMAX_DELAY );

    return nano_confirm_action_response;
}

bool nano_confirm_block(nl_block_t *head_block, nl_block_t *new_block) {
    /* IMPORTANT: Expects to be called from OUTSIDE the lvgl event loop. This 
     * function BLOCKS while waiting for user input.
     *
     * Reasoning: avoids requiring a large number of callbacks to perform simple
     *  logic flows.
     *
     * Prompts user to confirm transaction information before signing
     * Expects State Blocks 
     * Returns true on affirmation, false on error or cancellation.
     * */

    bool result = false;
    double display_amount;
    const char *title = "Confirm";


    if(head_block->type == STATE) {
        // Make sure the new_block's prev is the head_block
        {
            uint256_t head_block_hash;
            nl_block_compute_hash(head_block, head_block_hash);
            if(0 != memcmp(head_block_hash, new_block->previous, BIN_256)){
                goto exit;
            }
        }

        // Reject Invalid negative balances
        if(-1 == new_block->balance.s || -1 == head_block->balance.s){
            goto exit;
        }

        /******************************
         * Compute transaction amount *
         ******************************/
        {
            mbedtls_mpi transaction_amount;
            mbedtls_mpi_init(&transaction_amount);
            mbedtls_mpi_sub_mpi(&transaction_amount, &(head_block->balance), &(new_block->balance));
            if( E_SUCCESS != nl_mpi_to_nano_double(&transaction_amount, &display_amount) ){
                mbedtls_mpi_free(&transaction_amount);
                goto exit;
            }
            mbedtls_mpi_free(&transaction_amount);
        }

        char address[ADDRESS_BUF_LEN];
        char buf[200];
        if(0 != memcmp(head_block->representative, new_block->representative, BIN_256)){
            ESP_LOGI(TAG, "Detected Representative Change");
            /* Translate New Rep Address */
            if(E_SUCCESS != nl_public_to_address(address, sizeof(address),
                    new_block->representative)){
                goto exit;
            }
            snprintf(buf, sizeof(buf), "Change Rep to %s ?", address);
            if( nano_confirm_action(title, buf) ){
                result = true;
            }
            else{
                goto exit;
            }
        }
        if( display_amount > 0){
            ESP_LOGI(TAG, "Detected Send");
            /* Translate Destination Address */
            if(E_SUCCESS != nl_public_to_address(address, sizeof(address),
                    new_block->link)){
                goto exit;
            }

            snprintf(buf, sizeof(buf), 
                    "Send %."STR(CONFIG_JOLT_NANO_CONFIRM_DECIMALS)"lf NANO to %s ?",
                    display_amount, address);
            if( nano_confirm_action(title, buf) ){
                result = true;
            }
            else{
                goto exit;
            }
        }
        if( display_amount < 0){
            ESP_LOGI(TAG, "Detected Receive");
            // Auto Receive
            result = true;
        }
    }
    else if(head_block->type == UNDEFINED){
        ESP_LOGI(TAG, "No Frontier, verifying prevhash == 0");
        // new block must be an open state
        for(int i = 0; i < 32; i++){
            if( 0 != (new_block->previous)[i] ){
                goto exit;
            }
        }
        result = true;
    }
    else{
        ESP_LOGI(TAG, "Cannot verify with Legacy Block");
    }

exit:
    return result;
}

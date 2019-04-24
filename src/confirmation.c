/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */

#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "nano_helpers.h"
#include "confirmation.h"

static const char *TAG = "nano_conf";

#define STR_IMPL_(x) #x      //stringify argument
#define STR(x) STR_IMPL_(x)  //indirection to expand argument macros

static const char *title = "Confirm";

typedef struct confirm_obj_t{
    nl_block_t *head_block;
    nl_block_t *new_block;
    confirm_cb_t cb;
    void *param;
    double display_amount;
    bool is_send;
} confirm_obj_t;

static void send_cb( lv_obj_t *btn, lv_event_t event ) {
    if(LV_EVENT_SHORT_CLICKED == event || LV_EVENT_CANCEL == event) {
        confirm_obj_t *obj = jolt_gui_obj_get_param( btn );
        jolt_gui_scr_del();
        if( NULL != obj->cb ) {
            obj->cb(LV_EVENT_SHORT_CLICKED==event, obj->param);
        }
        free(obj);
    }
}


static void rep_change_cb( confirm_obj_t *obj ) {
    if( obj->is_send ){
        ESP_LOGI(TAG, "Detected Send");
        char address[ADDRESS_BUF_LEN];
        char buf[200];
        /* Translate Destination Address */
        if(E_SUCCESS != nl_public_to_address(address, sizeof(address), obj->new_block->link)){
            goto exit;
        }

        snprintf(buf, sizeof(buf), 
                "Send %."STR(CONFIG_JOLT_NANO_CONFIRM_DECIMALS)"lf NANO to %s ?",
                -obj->display_amount, address);

        lv_obj_t *scr = jolt_gui_scr_text_create(title, buf);
        jolt_gui_scr_set_event_cb(scr, send_cb);
        jolt_gui_scr_menu_set_param(scr, obj);
    }
    else {
        ESP_LOGI(TAG, "Detected Receive");
        /* Auto Receive, no need to prompt */
        if( NULL != obj->cb ) {
            obj->cb(true, obj->param);
        }
        free(obj);
    }

    return;

exit:
    if( NULL != obj->cb ) {
        obj->cb(false, obj->param);
    }
    if( NULL != obj ) {
        free(obj);
    }
}


static void rep_change_cb_helper( lv_obj_t *btn, lv_event_t event ) {
    if( LV_EVENT_SHORT_CLICKED == event ) {
        confirm_obj_t *obj = jolt_gui_obj_get_param( btn );
        jolt_gui_scr_del();
        rep_change_cb( obj );
    }
    else if( LV_EVENT_CANCEL == event ) {
		confirm_obj_t *obj = jolt_gui_obj_get_param( btn );
		jolt_gui_scr_del();
		if( NULL != obj->cb ) {
			obj->cb(false, obj->param);
		}
		free(obj);
    }
}

void nano_confirm_block(nl_block_t *head_block, nl_block_t *new_block, confirm_cb_t cb, void *param) {
     /*
     * Prompts user to confirm transaction information before signing
     * Expects State Blocks 
     * */
    confirm_obj_t *obj = NULL;

    obj = malloc(sizeof(confirm_obj_t));
    if( NULL == obj ) {
        ESP_LOGE(TAG, "Failed to allocate space for confirm_obj_t");
        goto exit;
    }
    obj->head_block = head_block;
    obj->new_block = new_block;
    obj->cb = cb;
    obj->param = param;
    obj->is_send = false;

    if(head_block->type == STATE) {
        /* Make sure the new_block's prev is the head_block */
        {
            uint256_t head_block_hash;
            nl_block_compute_hash(head_block, head_block_hash);
            if(0 != memcmp(head_block_hash, new_block->previous, BIN_256)){
                ESP_LOGE(TAG, "frontier and new_block's previous mismatch");
                goto exit;
            }
        }

        /* Reject Invalid negative balances */
        if(-1 == new_block->balance.s || -1 == head_block->balance.s){
            ESP_LOGW(TAG, "Cannot have a block with negative balance");
            goto exit;
        }

        /******************************
         * Compute transaction amount *
         ******************************/
        {
            mbedtls_mpi transaction_amount;
            mbedtls_mpi_init(&transaction_amount);
            mbedtls_mpi_sub_mpi(&transaction_amount, &(new_block->balance), &(head_block->balance));
            if( -1 == transaction_amount.s ) {
                obj->is_send = true;
            }
            if( E_SUCCESS != nl_mpi_to_nano_double(&transaction_amount, &obj->display_amount) ){
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
            lv_obj_t *scr = jolt_gui_scr_text_create(title, buf);
            jolt_gui_scr_set_active_param(scr, obj);
            jolt_gui_scr_set_event_cb(scr, rep_change_cb_helper); 
        }
        else {
            rep_change_cb( obj );
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
        /* No confirmation necessary for an Open Block */
        if( NULL != cb ) {
            cb(true, param);
        }
    }
    else{
        ESP_LOGI(TAG, "Cannot verify with Legacy Block");
        if( NULL != cb ) {
            cb(false, param);
        }
    }

    return;
exit:
    if( NULL != obj ) {
        free(obj);
    }
    if( NULL != cb ) {
        cb(false, param);
    }
}

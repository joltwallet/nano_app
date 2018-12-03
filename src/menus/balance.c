/* Jolt Wallet - Open Source Cryptocurrency Hardware Wallet
 Copyright (C) 2018  Brian Pugh, James Coxon, Michael Smaili
 https://www.joltwallet.com/
 */
#include "jolt_lib.h"
#include "nano_lib.h"
#include "esp_log.h"
#include "../nano_helpers.h"
#include "submenus.h"
#include "nano_parse.h"

#include "esp_http_client.h"


static const char TAG[] = "nano_balance";
static const char TITLE[] = "Nano Balance";


#if 0
static esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
            break;
        case HTTP_EVENT_ON_FINISH:
            ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
            break;
        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGD(TAG, "HTTP_EVENT_DISCONNECTED");
            break;
    }
    return ESP_OK;
}
static void https_async()
{
    esp_http_client_config_t config = {
        .url = "http://postman-echo.com/post",
        .event_handler = _http_event_handler,
        .is_async = true,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_err_t err;
    const char *post_data = "Using a Palantír requires a person with great strength of will and wisdom. The Palantíri were meant to "
                            "be used by the Dúnedain to communicate throughout the Realms in Exile. During the War of the Ring, "
                            "the Palantíri were used by many individuals. Sauron used the Ithil-stone to take advantage of the users "
                            "of the other two stones, the Orthanc-stone and Anor-stone, but was also susceptible to deception himself.";
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    while (1) { // keep checking status until it's ready
        err = esp_http_client_perform(client);
        if (err != ESP_ERR_HTTP_EAGAIN) {
            break;
        }
    }
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}


lv_action_t menu_nano_balance_cb( lv_obj_t *dummy ) {
    https_async();
    esp_http_client_config_t config = {
        .url = "http://yapraiwallet.space",
        .event_handler = _http_event_handler,
        .port = 5523,
        .is_async = true,
        .timeout_ms = 5000,
    };
    esp_http_client_handle_t client = esp_http_client_init( &config );
    esp_err_t err;
    const char *post_data = "{\"action\":\"block\",\"hash\":\"F851A853085146E53AA348D6AB5418096A0399F6DA90C37A2D2240CA17C2F00A\"}";

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));

    ESP_LOGI(TAG, "perform stuff %p", esp_http_client_perform);
    while (1) { // keep checking status until it's ready
        err = esp_http_client_perform(client);
        if (err != ESP_ERR_HTTP_EAGAIN) {
            break;
        }
    }
    ESP_LOGI(TAG, "done perform stuff");
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTPS Status = %d, content_length = %d",
                esp_http_client_get_status_code(client),
                esp_http_client_get_content_length(client));
    } else {
        ESP_LOGE(TAG, "Error perform http request %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
    return LV_RES_OK;
}
#endif

#if 1
lv_action_t menu_nano_balance_cb( lv_obj_t *dummy ) {
    double display_amount;

    lv_obj_t *scr = jolt_gui_scr_loading_create(TITLE);

    /*********************
     * Get My Public Key *
     *********************/
    uint256_t my_public_key;
    if( !nano_get_public(my_public_key) ) {
        return LV_RES_OK;
    }

    /********************************************
     * Get My Account's Frontier Block *
     ********************************************/
    // Assumes State Blocks Only
    // Outcome:
    //     * frontier_hash, frontier_block
    jolt_gui_scr_loading_update(scr, NULL, "Getting Frontier", 50);

    nl_block_t frontier_block;
    nl_block_init(&frontier_block);
    memcpy(frontier_block.account, my_public_key, sizeof(my_public_key));

    switch( nanoparse_web_frontier_block(&frontier_block) ){
        case E_SUCCESS:
            ESP_LOGI(TAG, "Successfully fetched frontier block");
            if( E_SUCCESS != nl_mpi_to_nano_double(&(frontier_block.balance),
                        &display_amount) ){
            }
            ESP_LOGI(TAG, "Approximate Account Balance: %0.3lf", display_amount);
            break;
        default:
            ESP_LOGI(TAG, "Failed to fetch frontier block (does it exist?)");
            display_amount = 0;
            break;
    }

    char buf[100];
    snprintf(buf, sizeof(buf), "%0.3lf Nano", display_amount);

    lv_obj_del(scr);
    jolt_gui_scr_text_create(TITLE, buf);

    return LV_RES_OK;
}
#endif

lv_action_t menu_nano_balance( lv_obj_t *btn ) {
    vault_refresh(NULL, menu_nano_balance_cb);
    return LV_RES_OK;
}

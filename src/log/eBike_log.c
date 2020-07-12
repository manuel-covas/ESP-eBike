#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <eBike_err.h>
#include <eBike_ble.h>
#include <sdkconfig.h>


char* log_data = NULL;
int log_index = -1;


eBike_err_t eBike_log_init() {
    eBike_err_t eBike_err;
    eBike_err.esp_err = ESP_OK;
    eBike_err.eBike_err_type = EBIKE_OK;

    if (log_data == NULL) {
        log_data = calloc(CONFIG_EBIKE_LOG_BUFFER_SIZE, sizeof(char));
        if (log_data == NULL) {
            eBike_err.eBike_err_type = EBIKE_LOG_INIT_MALLOC_FAIL;
        }else{
            log_index = 0;
        }
    }
    return eBike_err;
}


void eBike_log_add(char* message, uint16_t length) {
    if (log_data == NULL || log_index < 0 || message == NULL || length < 1 || ((log_index + length) > CONFIG_EBIKE_LOG_BUFFER_SIZE)) return;

    memcpy(log_data + log_index, message, length);
    log_index += length;
}

void eBike_log_clear() {
    if (log_data == NULL) return;

    memset(log_data, 0, CONFIG_EBIKE_LOG_BUFFER_SIZE);
    log_index = 0;
}


bool eBike_log_send() {

    if (log_data == NULL || log_index < 1) {
        printf("[LOG] - Not sending log. (%i bytes)\n", log_index);
        return true;
    }   printf("[LOG] - Sending log over BLE... (%i bytes)\n", log_index);
    
    eBike_err_t eBike_err = eBike_ble_tx((uint8_t*) log_data, log_index);

    if (eBike_err.eBike_err_type != EBIKE_OK) {
        printf("[LOG] - BLE send failed: %s\n", eBike_err_to_name(eBike_err.eBike_err_type));
    }else if (eBike_err.esp_err != ESP_OK) {
        printf("[LOG] - BLE send failed: %s\n", esp_err_to_name(eBike_err.esp_err));
    }else{
        printf("[LOG] - Sent log. Clearing.\n");
        eBike_log_clear();
    }
    return true;
}
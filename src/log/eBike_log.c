#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <eBike_err.h>
#include <sdkconfig.h>

char* log = NULL;
int log_index = NULL;


eBike_err_t eBike_ble_log_init() {
    eBike_err_t eBike_err;
    eBike_err.esp_err = ESP_OK;
    eBike_err.eBike_err_type = EBIKE_OK;

    if (log == NULL) {
        log = calloc(CONFIG_BLE_LOG_BUFFER_SIZE, sizeof(char));
        if (log == NULL) {
            eBike_err.eBike_err_type = EBIKE_BLE_LOG_INIT_MALLOC_FAIL;
        }else{
            log_index = 0;
        }
    }
    return eBike_err;
}

void eBike_ble_log(char* message, uint16_t length) {
    if (log == NULL || log_index == NULL || message == NULL || length == NULL || length < 1 || ((log_index + length) > CONFIG_BLE_LOG_BUFFER_SIZE)) return;

    memcpy(log + log_index, message, length - 1);
    log_index += length;
}
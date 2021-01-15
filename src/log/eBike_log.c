#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <eBike_err.h>
#include <eBike_ble.h>
#include <eBike_ble_io.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

char* log_data = NULL;
int log_index = -1;
bool log_inited = false;

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

    log_inited = eBike_err.eBike_err_type == EBIKE_OK;
    return eBike_err;
}


void eBike_log_add(char* message, uint16_t length) {
    printf(message);
    if (!log_inited || log_data == NULL || log_index < 0 || message == NULL || length < 1 || ((log_index + length) > CONFIG_EBIKE_LOG_BUFFER_SIZE)) return;
    
    memcpy(log_data + log_index, message, length);
    log_index += length;
}


int eBike_log_length() {
    return log_index;
}

void eBike_log_clear() {
    if (!log_inited || log_data == NULL) return;
    
    memset(log_data, 0, CONFIG_EBIKE_LOG_BUFFER_SIZE);
    log_index = 0;
}


void eBike_log_send(void* parameters) {
    eBike_err_t eBike_err;
    eBike_response_t response = {
        .eBike_response = EBIKE_COMMAND_LOG_RETRIEVE,
        .eBike_err = {
            .esp_err = ESP_OK,
            .eBike_err_type = EBIKE_OK
        }
    };

    if (!log_inited || log_data == NULL || log_index < 1) {

        printf("[Log] - Not sending log. (%i bytes)\n", log_index);
        eBike_err = eBike_ble_tx((uint8_t*) &response, sizeof(eBike_response_t));

        if (eBike_err.eBike_err_type != EBIKE_OK || eBike_err.esp_err != ESP_OK)
            printf("[Log] - BLE response send failed:\n"
                   "    ESP-eBike error: %s\n"
                   "    ESP_ERR: %s\n", eBike_err_to_name(eBike_err.eBike_err_type), esp_err_to_name(eBike_err.esp_err));

    }else{

        printf("[Log] - Sending BLE log... (%i bytes)\n", log_index);

        int sending_index = 0;
        while (sending_index < log_index) {

            int chunk_length = log_index - sending_index;
            if (chunk_length > 500) chunk_length = 500;
            
            uint8_t* response_buffer = malloc(sizeof(eBike_response_t) + chunk_length);
            if (response_buffer == NULL) {
                printf("[Log] - malloc(%i) failed!\n", sizeof(eBike_response_t) + chunk_length);
                goto eBike_clean;
            }
            
            memcpy(response_buffer, &response, sizeof(eBike_response_t));
            memcpy(response_buffer + sizeof(eBike_response_t), log_data + sending_index, chunk_length);
            sending_index += chunk_length;

            eBike_err = eBike_ble_tx(response_buffer, sizeof(eBike_response_t) + chunk_length);
            free(response_buffer);

            if (eBike_err.eBike_err_type != EBIKE_OK || eBike_err.esp_err != ESP_OK) {
                printf("[Log] - BLE response send failed:\n"
                       "    ESP-eBike error: %s\n"
                       "    ESP_ERR: %s\n", eBike_err_to_name(eBike_err.eBike_err_type), esp_err_to_name(eBike_err.esp_err));
                goto eBike_clean;
            }else{
                printf("[Log] - Sent %i bytes, %i left.\n", chunk_length, log_index - sending_index);
            }

            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
    }

    printf("[LOG] - Sent log. Clearing.\n");
    eBike_log_clear();

eBike_clean:
    printf("[Task] - Log task high water: %i\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelete(NULL);
}
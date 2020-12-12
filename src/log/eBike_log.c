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
        .eBike_command = EBIKE_COMMAND_LOG_RETRIEVE,
        .esp_err = ESP_OK,
        .eBike_err_type = EBIKE_OK
    };

    size_t response_header_buffer_len = 1 + sizeof(esp_err_t) + sizeof(eBike_err_type_t) + sizeof(int);
    uint8_t* response_header_buffer = malloc(response_header_buffer_len);

    response_header_buffer[0] = response.eBike_command;
    memcpy(response_header_buffer + 1, &response.esp_err, sizeof(esp_err_t));
    memcpy(response_header_buffer + 1 + sizeof(esp_err_t), &response.eBike_err_type, sizeof(eBike_err_type_t));
    memcpy(response_header_buffer + 1 + sizeof(esp_err_t) + sizeof(eBike_err_type_t), &log_index, sizeof(int));

    eBike_err = eBike_ble_tx(response_header_buffer, response_header_buffer_len);
    free(response_header_buffer);

    if (eBike_err.eBike_err_type != EBIKE_OK) {
        printf("[Log] - BLE response header send failed: %s\n", eBike_err_to_name(eBike_err.eBike_err_type));
        goto eBike_clean;
    }

    if (!log_inited || log_data == NULL || log_index < 1) {
        printf("[Log] - Not sending log. (%i bytes)\n", log_index);
        goto eBike_clean;
    }
    printf("[Log] - Sending log over BLE... (%i bytes)\n", log_index);
    
    int log_sending_index = 0;

    while (log_sending_index < log_index) {
        vTaskDelay(100 / portTICK_PERIOD_MS);
        uint16_t sending_length = log_index - log_sending_index;
        if (sending_length > 500) sending_length = 500;

        uint8_t* chunk = malloc(sending_length + 1);
        chunk[0] = EBIKE_COMMAND_LOG_RETRIEVE;
        memcpy(chunk + 1, log_data + log_sending_index, sending_length);

        eBike_err = eBike_ble_tx(chunk, sending_length + 1);
        log_sending_index += sending_length;
        free(chunk);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            printf("[Log] - BLE send failed: %s\n", eBike_err_to_name(eBike_err.eBike_err_type)); goto eBike_clean;
        }else if (eBike_err.esp_err != ESP_OK) {
            printf("[Log] - BLE send failed: %s\n", esp_err_to_name(eBike_err.esp_err)); goto eBike_clean;
        }else{
            printf("[Log] - Sent %i bytes, %i left.\n", sending_length, log_index - log_sending_index);
        }
    }

    printf("[LOG] - Sent log. Clearing.\n");
    eBike_log_clear();

eBike_clean:
    printf("[Task] - Log task high water: %i\n", uxTaskGetStackHighWaterMark(NULL));
    vTaskDelete(NULL);
}
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_gatts_api.h>
#include <eBike_err.h>
#include <eBike_ble_io.h>
#include <eBike_log.h>
#include <eBike_auth.h>


void eBike_ble_find_command(uint8_t* data, uint16_t length);
uint8_t* eBike_flip_array(uint8_t* flipped_data, uint16_t length);

uint8_t EBIKE_COMMAND_HEADER_HIGH = 0x29;
uint8_t EBIKE_COMMAND_HEADER_LOW  = 0x26;

eBike_command_state_t command_state = {
    .current_index = 0,
    .current_command = EBIKE_COMMAND_NONE
};

char* data_too_short_message = "[BLE] - Data sent was too short for command.\n";

void eBike_ble_io_recieve(void* p) {
    struct gatts_write_evt_param* parameters = (struct gatts_write_evt_param*) p;
    uint8_t* data = parameters->value;
    uint16_t length = parameters->len;
    
    if (data == NULL || length < 1) vTaskDelete(NULL);

    if (command_state.current_command == EBIKE_COMMAND_NONE) {
        eBike_ble_find_command(data, length);
    }else{
        return;
    }
    printf("[BLE] - Running command %i from index %i.\n", command_state.current_command, command_state.current_index);
    
    switch (command_state.current_command) {

        case EBIKE_COMMAND_LOG_RETRIEVE:
            xTaskCreate(eBike_log_send, "Log sender", 1900, NULL, tskIDLE_PRIORITY, NULL);
        break;
        
        case EBIKE_COMMAND_AUTH_GET_CHALLENGE:
            eBike_auth_get_challenge();
            eBike_ble_release_command_lock();
        break;

        case EBIKE_COMMAND_AUTH_SOLVE_CHALLENGE:
            if (length - command_state.current_index < (16 + 256)) {
                eBike_log_add(data_too_short_message, strlen(data_too_short_message));
                eBike_ble_release_command_lock();
                break;
            }
            if (eBike_auth_solve_challenge(data + command_state.current_index, 16, data + command_state.current_index + 16, 256)) {
                printf("[Auth] - Command is signed.\n");
            }else{
                printf("[Auth] - Command is not signed.\n");
            }
            eBike_ble_release_command_lock();
        break;

        default:
        break;
    }
}


void eBike_ble_find_command(uint8_t* data, uint16_t length) {
    if (length < 3) return;
    
    for (int i = 0; i < length - 1; i++) {
        if (memcmp(data + i,     &EBIKE_COMMAND_HEADER_HIGH, sizeof(uint8_t)) == 0 &&
            memcmp(data + i + 1, &EBIKE_COMMAND_HEADER_LOW,  sizeof(uint8_t)) == 0) {
            command_state.current_index = i + 3;
            command_state.current_command = *(data + i + 2);
            return;
        }
    }
}

void eBike_ble_release_command_lock() {
    command_state.current_command = EBIKE_COMMAND_NONE;
    command_state.current_index = 0;
}
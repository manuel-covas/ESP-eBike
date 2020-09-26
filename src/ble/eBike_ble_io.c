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


char* data_too_short_message = "[BLE] - Data sent was too short for command.\n";

bool command_locked = false;

void eBike_ble_io_recieve(void* p) {

    if (command_locked) return;
    command_locked = true;

    struct gatts_write_evt_param* parameters = (struct gatts_write_evt_param*) p;
    uint8_t* data = parameters->value;
    uint16_t length = parameters->len;
    
    if (data == NULL || length < 1) return;
    
    switch (*data) {

        case EBIKE_COMMAND_LOG_RETRIEVE:
            xTaskCreate(eBike_log_send, "Log sender", 1900, NULL, tskIDLE_PRIORITY, NULL);
        break;
        
        case EBIKE_COMMAND_AUTH_GET_CHALLENGE:
            eBike_auth_get_challenge();
            eBike_ble_release_command_lock();
        break;

        case EBIKE_COMMAND_AUTH_SOLVE_CHALLENGE:
            if (length - 1 < (16 + 256)) {
                eBike_log_add(data_too_short_message, strlen(data_too_short_message));
                eBike_ble_release_command_lock();
                break;
            }
            if (eBike_auth_solve_challenge(data + 1, 16, data + 17, 256)) {
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


void eBike_ble_release_command_lock() {
    
}
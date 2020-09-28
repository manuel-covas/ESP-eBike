#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_gatts_api.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_ble_io.h>
#include <eBike_auth.h>
#include <eBike_nvs.h>

void eBike_ble_execute_authed_command(eBike_authed_command_t authed_command);

char* data_too_short_message = "[BLE] - Data sent was too short for command.\n";
char* command_signature_inavlid = "[BLE] - Authed command signature verification failed.\n";

bool command_locked = false;

void eBike_ble_io_recieve(void* p) {

    if (command_locked) return;
    command_locked = true;

    struct gatts_write_evt_param* parameters = (struct gatts_write_evt_param*) p;
    uint8_t* data = parameters->value;
    uint16_t length = parameters->len;
    
    if (data == NULL || length < 1) goto too_short;
    
    switch (*data) {

        case EBIKE_COMMAND_LOG_RETRIEVE:
            xTaskCreate(eBike_log_send, "Log sender", 1900, NULL, tskIDLE_PRIORITY, NULL);
            command_locked = false;
        break;
        
        case EBIKE_COMMAND_AUTH_GET_CHALLENGE:
            eBike_auth_get_challenge();
            command_locked = false;
        break;

        case EBIKE_COMMAND_AUTHED_COMMAND:
            
            eBike_authed_command_t authed_command;
            
            if (length < 2) goto too_short;
            authed_command.authed_command_length = *(data + 1);

            if (length < 2 + authed_command.authed_command_length) goto too_short;
            authed_command.authed_command = data + 2;

            if (length < 2 + authed_command.authed_command_length + 2) goto too_short;
            authed_command.signature_length = *(data + 2 + authed_command.authed_command_length);
            
            if (length < 2 + authed_command.authed_command_length + 2 + authed_command.signature_length) goto too_short;
            authed_command.signature = *(data + 2 + authed_command.authed_command_length + 2);
            
            if (eBike_auth_solve_challenge(authed_command.authed_command, authed_command.authed_command_length, authed_command.signature, authed_command.signature_length)) {
                printf("[BLE] - Authed command verified, executing...\n");
                eBike_ble_execute_authed_command(authed_command);
            }else{
                eBike_log_add(command_signature_inavlid, strlen(command_signature_inavlid));
            }
            command_locked = false;
        break;

        default:
            char* message;
            asprintf(&message, "[BLE] - Unkown command %02X.\n", *data);
            eBike_log_add(message, strlen(message));
            free(message);
            command_locked = false;
        break;
    }

    return;

too_short:
    eBike_log_add(data_too_short_message, strlen(data_too_short_message));
    command_locked = false;
}


void eBike_ble_execute_authed_command(eBike_authed_command_t authed_command) {

    switch (*authed_command.authed_command) {

        case EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS:
            if (authed_command.authed_command_length < (sizeof(eBike_settings_t) + 1))
                goto too_short;
            eBike_nvs_settings_put((authed_command.authed_command + 1));
        break;

        default:
            char* message;
            asprintf(&message, "[BLE] - Unknown command %02X.\n", *authed_command.authed_command);
            eBike_log_add(message, strlen(message));
            free(message);
            command_locked = false;
        break;
    }

too_short:
    eBike_log_add(data_too_short_message, strlen(data_too_short_message));
}
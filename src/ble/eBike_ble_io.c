#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_gatts_api.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_util.h>
#include <eBike_ble.h>
#include <eBike_ble_io.h>
#include <eBike_auth.h>
#include <eBike_nvs.h>

void eBike_ble_execute_authed_command(eBike_authed_command_t authed_command);
void eBike_ble_send_command_response(eBike_command_response_t command_response, uint8_t* response_data, size_t response_data_length);


char* data_too_short_message = "[BLE] - Data sent was too short for command.\n";
char* command_signature_inavlid = "[BLE] - Authed command signature verification failed.\n";

bool command_locked = false;



void eBike_ble_io_recieve(void* p) {

    if (command_locked) return;
    command_locked = true;

    struct gatts_write_evt_param* parameters = (struct gatts_write_evt_param*) p;
    uint8_t* data = parameters->value;
    uint16_t length = parameters->len;
    eBike_command_response_t response;
    
    if (data == NULL || length < 1) goto too_short;
    

    switch (*data) {

        case EBIKE_COMMAND_LOG_RETRIEVE:
            xTaskCreate(eBike_log_send, "Log sender", 1900, NULL, tskIDLE_PRIORITY, NULL);
            command_locked = false;
        break;


        case EBIKE_COMMAND_GET_SETTINGS:
            ;
            eBike_settings_t eBike_settings;
            eBike_err_t eBike_err = eBike_nvs_settings_get(&eBike_settings);

            memset(&response, 0, sizeof(eBike_command_response_t));
            response.eBike_command = EBIKE_COMMAND_GET_SETTINGS;
            response.eBike_err_type = eBike_err.eBike_err_type;
            response.esp_err = eBike_err.esp_err;
            
            if (eBike_err.eBike_err_type == EBIKE_OK) {
                eBike_ble_send_command_response(response, (uint8_t*)&eBike_settings, sizeof(eBike_settings_t));
            }else{
                eBike_ble_send_command_response(response, NULL, 0);
            }

            command_locked = false;
        break;
        

        case EBIKE_COMMAND_AUTH_GET_CHALLENGE:
            
            memset(&response, 0, sizeof(eBike_command_response_t));
            response.eBike_command = EBIKE_COMMAND_AUTH_GET_CHALLENGE;
            response.eBike_err_type = EBIKE_OK;
            response.esp_err = ESP_OK;

            eBike_ble_send_command_response(response, eBike_auth_get_challenge(), CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH);
            printf("[Auth] - Current challenge: ");
            bytes_to_hex(eBike_auth_get_challenge(), CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH);
            printf("\n");

            command_locked = false;
        break;


        case EBIKE_COMMAND_AUTHED_COMMAND:
            ;
            eBike_authed_command_t authed_command;
            
            if (length < 2) goto too_short;
            authed_command.authed_command_length = *(data + 1);

            if (length < 2 + authed_command.authed_command_length) goto too_short;
            authed_command.authed_command = data + 2;

            if (length < 2 + authed_command.authed_command_length + 2) goto too_short;
            authed_command.signature_length = *((uint16_t*)(data + 2 + authed_command.authed_command_length));
            
            if (length < 2 + authed_command.authed_command_length + 2 + authed_command.signature_length) goto too_short;
            authed_command.signature = (data + 2 + authed_command.authed_command_length + 2);
            
            if (eBike_auth_solve_challenge(authed_command.authed_command, authed_command.authed_command_length, authed_command.signature, authed_command.signature_length)) {
                printf("[BLE] - Authed command verified, executing...\n");
                eBike_ble_execute_authed_command(authed_command);
            }else{
                eBike_log_add(command_signature_inavlid, strlen(command_signature_inavlid));
            }
            command_locked = false;
        break;


        default:
            ;
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

    uint8_t* response;
    char* message;

    switch (*authed_command.authed_command) {

        case EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS:
            ;
            size_t length = (sizeof(eBike_settings_t) + 1);

            if (authed_command.authed_command_length < length)
                goto too_short;
            
            eBike_settings_t settings = *(eBike_settings_t*) (authed_command.authed_command + 1);
            eBike_err_t eBike_err = eBike_nvs_settings_put(&settings);

            response = malloc(3);
            response[0] = EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS;
            response[1] = eBike_err.esp_err;
            response[2] = eBike_err.eBike_err_type;

            eBike_ble_tx(response, 3);
            free(response);

            if (eBike_err.eBike_err_type != EBIKE_OK) {
                asprintf(&message, "[NVS] - Saving of NVS settings failed: eBike_err: %s (%i) esp_err: %s (%i)\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err);
            }else{
                asprintf(&message, "[NVS] - Saved NVS settings.\n");
            }
            eBike_log_add(message, strlen(message));
            free(message);
            
            command_locked = false;
        break;

        default:
            
            asprintf(&message, "[BLE] - Unknown command %02X.\n", *authed_command.authed_command);
            eBike_log_add(message, strlen(message));
            free(message);

            command_locked = false;
        break;
    }

    return;

too_short:
    eBike_log_add(data_too_short_message, strlen(data_too_short_message));
}


void eBike_ble_send_command_response(eBike_command_response_t command_response, uint8_t* response_data, size_t response_data_length) {

    size_t response_length = 1 + sizeof(eBike_err_type_t) + sizeof(esp_err_t) + response_data_length;
    uint8_t* response = malloc(response_length);

    response[0] = command_response.eBike_command;
    memcpy(response + 1, &command_response.eBike_err_type, sizeof(eBike_err_type_t));
    memcpy(response + 1 + sizeof(eBike_err_type_t), &command_response.esp_err, sizeof(esp_err_t));
    
    if (response_data_length > 0)
        memcpy(response + 1 + sizeof(eBike_err_type_t) + sizeof(esp_err_t), response_data, response_data_length);

    eBike_ble_tx(response, response_length);
    free(response);
}
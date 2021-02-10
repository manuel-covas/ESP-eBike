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
#include <eBike_bms.h>
#include <eBike_gpio.h>
#include <bq76930.h>

void eBike_ble_execute_authed_command(eBike_authed_command_t authed_command);

char* data_too_short_message = "[BLE] - Data sent was too short for command.\n";
char* unknown_command_message = "[BLE] - Unknown command %02X.\n";
char* unknown_authed_command_message = "[BLE] - Unknown authenticated command %02X.\n";
char* command_signature_inavlid = "[BLE] - Authed command signature verification failed.\n";

bool system_stats_stream_enabled = false;


void eBike_ble_io_recieve(struct gatts_write_evt_param* p) {

    struct gatts_write_evt_param* parameters = (struct gatts_write_evt_param*) p;
    uint8_t* data = parameters->value;
    uint16_t data_length = parameters->len;

    char* message;
    eBike_response_t response = {
        .eBike_err = {
            .eBike_err_type = EBIKE_OK,
            .esp_err = ESP_OK
        }
    };
    
    if (data == NULL || data_length < 1) return;
    response.eBike_response = *data;


    switch (response.eBike_response) {

        case EBIKE_COMMAND_LOG_RETRIEVE:
            eBike_log_send();
        break;

        case EBIKE_COMMAND_GET_SETTINGS:
            ;
            eBike_settings_t eBike_settings;
            response.eBike_err = eBike_nvs_settings_get(&eBike_settings);
            
            if (response.eBike_err.eBike_err_type == EBIKE_OK) {
                eBike_queue_ble_message(&response, &eBike_settings, sizeof(eBike_settings_t), true);
            }else{
                eBike_queue_ble_message(&response, NULL, 0, true);
            }
        break;


        case EBIKE_COMMAND_GET_ADC_CHARACTERISTICS:
            ;
            bq76930_adc_characteristics_t adc_characteristics = bq76930_get_adc_characteristics();
            eBike_queue_ble_message(&response, &adc_characteristics, sizeof(bq76930_adc_characteristics_t), true);
        break;


        case EBIKE_COMMAND_SYSTEM_STATS_STREAM:
            
            if (data_length < 2)
                goto too_short;

            system_stats_stream_enabled = *(data + 1) != 0;
            eBike_queue_ble_message(&response, NULL, 0, true);
        break;


        case EBIKE_COMMAND_AUTH_GET_CHALLENGE:
            eBike_queue_ble_message(&response, eBike_auth_get_challenge(), CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH, true);
            printf("[Auth] - Current challenge: ");
            bytes_to_hex(eBike_auth_get_challenge(), CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH);
            printf("\n");
        break;


        case EBIKE_COMMAND_AUTHED_COMMAND:
            ;
            eBike_authed_command_t authed_command;
            
            if (data_length < 3)
                goto too_short;

            authed_command.length = *((uint16_t*)(data + 1));
            
            if (authed_command.length < 1 || data_length < 3 + authed_command.length + 2)
                goto too_short;

            authed_command.command = data + 3;
            authed_command.signature_length = *((uint16_t*)(authed_command.command + authed_command.length));

            if (data_length < 3 + authed_command.length + 2 + authed_command.signature_length)
                goto too_short;
            
            authed_command.signature = data + 3 + authed_command.length + 2;
            
            if (eBike_auth_solve_challenge(authed_command.command, authed_command.length, authed_command.signature, authed_command.signature_length)) {
                eBike_ble_execute_authed_command(authed_command);
            }else{
                response.eBike_err.eBike_err_type = EBIKE_AUTHED_COMMAND_FAIL;
                eBike_log_add(command_signature_inavlid, strlen(command_signature_inavlid));
            }

            eBike_queue_ble_message(&response, NULL, 0, true);
            return;
            
        break;


        default:
            response.eBike_err.eBike_err_type = EBIKE_BLE_COMMAND_UNKNOWN;
            eBike_queue_ble_message(&response, NULL, 0, true);

            asprintf(&message, unknown_command_message, response.eBike_response);
            eBike_log_add(message, strlen(message));
            free(message);
        break;

too_short:
        response.eBike_err.eBike_err_type = EBIKE_BLE_COMMAND_TOO_SHORT;
        eBike_log_add(data_too_short_message, strlen(data_too_short_message));
        eBike_queue_ble_message(&response, NULL, 0, true);
    }

    return;
}


void eBike_ble_execute_authed_command(eBike_authed_command_t authed_command) {

    eBike_response_t response = {
        .eBike_response = *authed_command.command,
        .eBike_err = {
            .eBike_err_type = EBIKE_OK,
            .esp_err = ESP_OK
        }
    };
    
    switch (response.eBike_response) {

        case EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS:
            
            if (authed_command.length < 1 + sizeof(eBike_settings_t))
                goto too_short;

            eBike_settings_t eBike_settings = *((eBike_settings_t*) (authed_command.command + 1));
            eBike_err_t eBike_err = eBike_nvs_settings_put(&eBike_settings);

            char* message = eBike_print_settings(eBike_settings, "[System] - Saving settings form bluetooth:");
            eBike_log_add(message, strlen(message));
            free(message);

            if (eBike_err.eBike_err_type != EBIKE_OK) {
                
                response.eBike_err = eBike_err;
                eBike_queue_ble_message(&response, NULL, 0, true);
                
                asprintf(&message, "[NVS] - Saving of NVS settings failed: eBike_err: %s (%i) esp_err: %s (%i)\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err);
                eBike_log_add(message, strlen(message));
                free(message);
                return;
            }
            
            eBike_err = eBike_bms_config(eBike_settings);
            response.eBike_err = eBike_err;
            eBike_queue_ble_message(&response, NULL, 0, true);

            if (eBike_err.eBike_err_type == EBIKE_OK) {
                asprintf(&message, "[NVS] - Saved NVS settings.\n"
                                   "[BMS] - Applied new settings.\n");
            }else{
                asprintf(&message, "[NVS] - Saved NVS settings.\n"
                                   "[BMS] - Failed to apply new settings: eBike_err: %s (%i) esp_err: %s (%i)\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err);
            }

            eBike_log_add(message, strlen(message));
            free(message);
            return;
        break;


        case EBIKE_COMMAND_AUTHED_COMMAND_TOGGLE_UNLOCK:

            if (authed_command.length < 2)
                goto too_short;
            
            bool unlock = *(authed_command.command + 1) == 0x01;

            printf("[GPIO] - %s PWM ESC control signal...\n", unlock ? "Enabling" : "Disabling");
            eBike_err = eBike_gpio_pwm_toggle(unlock);
            
            if (eBike_err.eBike_err_type != EBIKE_OK) {
                asprintf(&message, "[GPIO] - Failed to %s PWM ESC control signal: eBike_err: %s (%i) esp_err: %s (%i)\n",
                                   unlock ? "enable" : "disable",
                                   eBike_err_to_name(eBike_err.eBike_err_type),
                                   eBike_err.eBike_err_type,
                                   esp_err_to_name(eBike_err.esp_err),
                                   eBike_err.esp_err);
                eBike_log_add(message, strlen(message));
                free(message);

                response.eBike_err = eBike_err;
                eBike_queue_ble_message(&response, NULL, 0, true);
            }

            bq76930_sys_ctrl_2_t sys_ctrl_2 = {
                .disable_delays = false,
                .coulomb_counter_enable = true,
                .coulomb_counter_oneshot = false,
                .discharge_on = unlock,
                .charge_on = unlock
            };

            printf("[BMS] - %s...\n", unlock ? "Unlocking" : "Locking");
            eBike_err = bq76930_write_register(BQ76930_SYS_CTRL_2, (uint8_t*) &sys_ctrl_2);
            
            if (eBike_err.eBike_err_type != EBIKE_OK) {
                asprintf(&message, "[BMS] - Failed to %s: eBike_err: %s (%i) esp_err: %s (%i)\n",
                                   unlock ? "unlock" : "lock",
                                   eBike_err_to_name(eBike_err.eBike_err_type),
                                   eBike_err.eBike_err_type,
                                   esp_err_to_name(eBike_err.esp_err),
                                   eBike_err.esp_err);
                eBike_log_add(message, strlen(message));
                free(message);

                response.eBike_err = eBike_err;
                eBike_queue_ble_message(&response, NULL, 0, true);
            }
            
        break;

        default:
            response.eBike_err.eBike_err_type = EBIKE_BLE_AUTHED_COMMAND_UNKNOWN;
            eBike_queue_ble_message(&response, NULL, 0, true);

            asprintf(&message, unknown_authed_command_message, response.eBike_response);
            eBike_log_add(message, strlen(message));
            free(message);
        break;

too_short:
        response.eBike_err.eBike_err_type = EBIKE_BLE_COMMAND_TOO_SHORT;
        eBike_queue_ble_message(&response, NULL, 0, true);
        eBike_log_add(data_too_short_message, strlen(data_too_short_message));
    }

    return;
}
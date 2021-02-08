#ifndef EBIKE_ERR_H
#define EBIKE_ERR_H

#include <esp_err.h>

#define EBIKE_ERROR_BEEP_LONG_MS 150
#define EBIKE_ERROR_BEEP_SHORT_MS 80

typedef enum {
    EBIKE_OK = 0,                                 // Success.
    EBIKE_NVS_INIT_ERASE_FAIL,                    // NVS Init: Failed to erase flash chip.
    EBIKE_NVS_INIT_FAIL,                          // NVS Init: ESP-IDF nvs_flash_init() failed.
    EBIKE_NVS_INIT_OPEN_FAIL,                     // NVS Init: Failed to open NVS namespace EBIKE_NVS_NAMESPACE with mode NVS_READWRITE.
    EBIKE_GPIO_INIT_CONFIG_FAIL,                  // GPIO Init: ESP-IDF gpio_config() failed.
    EBIKE_GPIO_INIT_PWM_CONFIG_FAIL,              // GPIO Init: Failed to configure PWM (LEDC).
    EBIKE_GPIO_PWM_TOGGLE_FAIL,                   // GPIO: Failed to toggle PWM (LEDC).
    EBIKE_GPIO_PWM_SET_DUTY_FAIL,                 // GPIO: Failed to set duty cycle of PWM (LEDC).
    EBIKE_ADC_INIT_SET_WIDTH_FAIL,                // ADC Init: Failed to set ADC1 reading width.
    EBIKE_ADC_INIT_SET_ATTEN_FAIL,                // ADC Init: Failed to set ADC1 throttle channel attenuation.
    EBIKE_BLE_INIT_OUTGOING_QUEUE_CREATE_FAIL,    // BLE Init: Outgoing message queue creation failed.
    EBIKE_BLE_INIT_OUTGOING_TASK_CREATE_FAIL,     // BLE Init: Outgoing message task creation failed.
    EBIKE_BLE_INIT_CONTROLLER_INIT_FAIL,          // BLE Init: Initializing bluetooth controller with BT_CONTROLLER_INIT_CONFIG_DEFAULT failed.
    EBIKE_BLE_INIT_ENABLE_CONTROLLER_FAIL,        // BLE Init: Bluetooth controller enabling failed.
    EBIKE_BLE_INIT_BLUEDROID_INIT_FAIL,           // BLE Init: Bluedroid stack initialization failed.
    EBIKE_BLE_INIT_BLUEDROID_ENABLE_FAIL,         // BLE Init: Bluedroid stack enabling failed.
    EBIKE_BLE_INIT_GAP_CALLBACK_REGISTER_FAIL,    // BLE Init: GAP callback function registration failed.
    EBIKE_BLE_INIT_GATTS_CALLBACK_REGISTER_FAIL,  // BLE Init: GATT server callback function registration failed.
    EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL,       // BLE Init: GATT server callback function registration failed.
    EBIKE_BLE_INIT_SET_BT_NAME_FAIL,              // BLE Init: Failed to set device's bluetooth name.
    EBIKE_BLE_INIT_SET_ADV_DATA_FAIL,             // BLE Init: Failed to set desired BLE advertising data.
    EBIKE_BLE_INIT_START_ADV_FAIL,                // BLE Init: Failed to start BLE advertising.
    EBIKE_BLE_TX_NOT_CONNECTED,                   // BLE Transmit: An attempt to send data over BLE was made but no connection was active.
    EBIKE_BLE_TX_BAD_ARGUMENTS,                   // BLE Transmit: Incorrect parameters passed to eBike_ble_tx()
    EBIKE_BLE_TX_MALLOC_FAIL,                     // BLE Transmit: Malloc failed in eBike_queue_ble_message()
    EBIKE_BLE_TX_QUEUE_FAIL,                      // BLE Transmit: Failed to add BLE message to outgoing queue.
    EBIKE_BLE_COMMAND_TOO_SHORT,                  // BLE Command: Received data was too short.
    EBIKE_BLE_COMMAND_UNKNOWN,                    // BLE Command: Unknown command.
    EBIKE_BLE_AUTHED_COMMAND_UNKNOWN,             // BLE Command: Unknown authenticated command.
    EBIKE_LOG_INIT_MALLOC_FAIL,                   // BLE Log Init: malloc for the log's buffer failed.
    EBIKE_LOG_TX_MALLOC_FAIL,                     // BLE Log Transmit: malloc for a log chunk failed in eBike_log_send()
    EBIKE_AUTH_INIT_MALLOC_FAIL,                  // Authentication Init: Malloc failed
    EBIKE_AUTH_INIT_PARSE_KEY_FAIL,               // Authentication Init: Parsing of built in public key failed.
    EBIKE_AUTHED_COMMAND_FAIL,                    // Authentication: Signature verification failed.
    EBIKE_NVS_SETTINGS_GET_FAIL,                  // NVS Settings: Read from NVS failed.
    EBIKE_NVS_SETTINGS_PUT_FAIL,                  // NVS Settings: Write to NVS failed.
    EBIKE_NVS_SETTINGS_CRC_MISMATCH,              // NVS Settings: CRC check failed. Could happen when reading or writing settings.
    EBIKE_BMS_INIT_I2C_CONFIG_FAIL,               // BMS Init: I2C driver configuring failed.
    EBIKE_BMS_INIT_I2C_INSTALL_FAIL,              // BMS Init: I2C driver activation failed.
    EBIKE_BMS_I2C_BUILD_COMMAND_FAIL,             // BMS I2C Communication: Failure while preparing I2C command to communicate with BQ769x0.
    EBIKE_BMS_I2C_COMMAND_FAIL,                   // BMS I2C Communication: I2C data exchange failed. (not acknowledged or other)
    EBIKE_BMS_I2C_CRC_MISMATCH                    // BMS I2C Communication: BQ769x0 communication CRC mismatched. (explained in the chip's datasheet)
} eBike_err_type_t;

typedef struct eBike_err_t {
    esp_err_t esp_err;
    eBike_err_type_t eBike_err_type;
}
eBike_err_t;


#define EBIKE_HANDLE_ERROR(esp_err_param, eBike_err_type_param, eBike_err_param) \
    if (1) {                                                                     \
        esp_err_t result_err = esp_err_param;                                    \
        if (result_err != ESP_OK) {                                              \
            eBike_err_param.esp_err = result_err;                                \
            eBike_err_param.eBike_err_type = eBike_err_type_param;               \
            goto eBike_clean;                                                    \
        }else{                                                                   \
            eBike_err_param.esp_err = ESP_OK;                                    \
            eBike_err_param.eBike_err_type = EBIKE_OK;                           \
        }                                                                        \
    }

const char* eBike_err_to_name(eBike_err_type_t);
void eBike_err_report(eBike_err_t err);
void eBike_beep(uint32_t* duration_ms);

#endif
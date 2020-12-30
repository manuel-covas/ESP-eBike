#ifndef EBIKE_ERR_H
#define EBIKE_ERR_H

#include <esp_err.h>

#define EBIKE_ERROR_BEEP_LONG_MS 150
#define EBIKE_ERROR_BEEP_SHORT_MS 80

#define EBIKE_OK 0                                      // Success.
#define EBIKE_NVS_INIT_ERASE_FAIL 1                     // NVS Init: Failed to erase flash chip.
#define EBIKE_NVS_INIT_FAIL 2                           // NVS Init: ESP-IDF nvs_flash_init() failed.
#define EBIKE_NVS_INIT_OPEN_FAIL 3                      // NVS Init: Failed to open NVS namespace EBIKE_NVS_NAMESPACE with mode NVS_READWRITE.
#define EBIKE_GPIO_INIT_CONFIG_FAIL 4                   // GPIO Init: ESP-IDF gpio_config() failed.
#define EBIKE_BLE_INIT_CONTROLLER_INIT_FAIL 5           // BLE Init: Initializing bluetooth controller with BT_CONTROLLER_INIT_CONFIG_DEFAULT failed.
#define EBIKE_BLE_INIT_ENABLE_CONTROLLER_FAIL 6         // BLE Init: Bluetooth controller enabling failed.
#define EBIKE_BLE_INIT_BLUEDROID_INIT_FAIL 7            // BLE Init: Bluedroid stack initialization failed.
#define EBIKE_BLE_INIT_BLUEDROID_ENABLE_FAIL 8          // BLE Init: Bluedroid stack enabling failed.
#define EBIKE_BLE_INIT_GAP_CALLBACK_REGISTER_FAIL 9     // BLE Init: GAP callback function registration failed.
#define EBIKE_BLE_INIT_GATTS_CALLBACK_REGISTER_FAIL 10  // BLE Init: GATT server callback function registration failed.
#define EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL 11       // BLE Init: GATT server callback function registration failed.
#define EBIKE_BLE_INIT_SET_BT_NAME_FAIL 12              // BLE Init: Failed to set device's bluetooth name.
#define EBIKE_BLE_INIT_SET_ADV_DATA_FAIL 13             // BLE Init: Failed to set desired BLE advertising data.
#define EBIKE_BLE_INIT_START_ADV_FAIL 14                // BLE Init: Failed to start BLE advertising.
#define EBIKE_BLE_TX_NOT_CONNECTED 15                   // BLE Transmit: An attempt to send data over BLE was made but no connection was active.
#define EBIKE_BLE_TX_BAD_ARGUMENTS 16                   // BLE Transmit: Incorrect parameters passed to eBike_ble_tx()
#define EBIKE_LOG_INIT_MALLOC_FAIL 17                   // BLE Log Init: Failed to malloc for the log's buffer failed.
#define EBIKE_AUTH_INIT_MALLOC_FAIL 18                  // Authentication Init: Malloc failed
#define EBIKE_AUTH_INIT_PARSE_KEY_FAIL 19               // Authentication Init: Parsing of built in public key failed.
#define EBIKE_NVS_SETTINGS_GET_FAIL 20                  // NVS Settings: Read from NVS failed.
#define EBIKE_NVS_SETTINGS_PUT_FAIL 21                  // NVS Settings: Write to NVS failed.
#define EBIKE_NVS_SETTINGS_CRC_MISMATCH 22              // NVS Settings: CRC check failed. Could happen when reading or writing settings.
#define EBIKE_BMS_INIT_I2C_CONFIG_FAIL 23               // BMS Init: I2C driver configuring failed.
#define EBIKE_BMS_INIT_I2C_INSTALL_FAIL 24              // BMS Init: I2C driver activation failed.
#define EBIKE_BMS_I2C_BUILD_COMMAND_FAIL 25             // BMS I2C Communication: Failure while preparing I2C command to communicate with BQ769x0.
#define EBIKE_BMS_I2C_COMMAND_FAIL 26                   // BMS I2C Communication: I2C data exchange failed. (not acknowledged or other)
#define EBIKE_BMS_I2C_CRC_MISMATCH 27                   // BMS I2C Communication: BQ769x0 communication CRC mismatched. (explained in the chip's datasheet)

typedef int32_t eBike_err_type_t;


typedef struct eBike_err_t {
    esp_err_t esp_err;
    eBike_err_type_t eBike_err_type;
}
eBike_err_t;


#define EBIKE_HANDLE_ERROR(esp_err_param, eBike_err_type_param, eBike_err_param) \
    if (esp_err_param != ESP_OK) {                                               \
        eBike_err_param.esp_err = esp_err_param;                                 \
        eBike_err_param.eBike_err_type = eBike_err_type_param;                   \
        goto eBike_clean;                                                  \
    }else{                                                                       \
        eBike_err_param.esp_err = ESP_OK;                                        \
        eBike_err_param.eBike_err_type = EBIKE_OK;                         \
    }

const char* eBike_err_to_name(eBike_err_type_t);
void eBike_err_report(eBike_err_t err);
void eBike_beep(uint32_t* duration_ms);

#endif
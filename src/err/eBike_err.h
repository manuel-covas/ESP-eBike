#ifndef EBIKE_ERR_H
#define EBIKE_ERR_H

#include <esp_err.h>

#define EBIKE_ERROR_BEEP_LONG_MS 150
#define EBIKE_ERROR_BEEP_SHORT_MS 80

typedef enum {
    EBIKE_OK,
    EBIKE_NVS_INIT_ERASE_FAIL,
    EBIKE_NVS_INIT_FAIL,
    EBIKE_NVS_INIT_OPEN_FAIL,
    EBIKE_GPIO_INIT_CONFIG_FAIL,
    EBIKE_BLE_INIT_CONTROLLER_INIT_FAIL,
    EBIKE_BLE_INIT_ENABLE_CONTROLLER_FAIL,
    EBIKE_BLE_INIT_BLUEDROID_INIT_FAIL,
    EBIKE_BLE_INIT_BLUEDROID_ENABLE_FAIL,
    EBIKE_BLE_INIT_GAP_CALLBACK_REGISTER_FAIL,
    EBIKE_BLE_INIT_GATTS_CALLBACK_REGISTER_FAIL,
    EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL,
    EBIKE_BLE_INIT_GATTS_LOCAL_MTU_SET_FAIL,
    EBIKE_BLE_INIT_SET_BT_NAME_FAIL,
    EBIKE_BLE_INIT_SET_ADV_DATA_FAIL,
    EBIKE_BLE_INIT_START_ADV_FAIL,
    EBIKE_BLE_TX_NOT_CONNECTED,
    EBIKE_BLE_TX_BAD_ARGUMENTS,
    EBIKE_LOG_INIT_MALLOC_FAIL,
    EBIKE_AUTH_INIT_MALLOC_FAIL,
    EBIKE_AUTH_INIT_PARSE_KEY_FAIL,
    EBIKE_NVS_SETTINGS_GET_FAIL,
    EBIKE_NVS_SETTINGS_CRC_MISMATCH,
    EBIKE_BMS_INIT_I2C_CONFIG_FAIL,
    EBIKE_BMS_INIT_I2C_INSTALL_FAIL,
    EBIKE_BMS_I2C_BUILD_COMMAND_FAIL,
    EBIKE_BMS_I2C_COMMAND_FAIL,
    EBIKE_BMS_I2C_CRC_MISMATCH
}
eBike_err_type_t;


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
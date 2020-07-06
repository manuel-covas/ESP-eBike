#ifndef EBIKE_ERR_H
#define EBIKE_ERR_H

#include <esp_err.h>

typedef enum {
    EBIKE_NO_ERROR,
    EBIKE_NVS_INIT_ERASE_FAIL,
    EBIKE_NVS_INIT_FAIL,
    EBIKE_NVS_INIT_OPEN_FAIL,
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
    EBIKE_BLE_INIT_START_ADV_FAIL
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
        return eBike_err_param;                                                  \
    }else{                                                                       \
        eBike_err_param.esp_err = ESP_OK;                                        \
        eBike_err_param.eBike_err_type = EBIKE_NO_ERROR;                         \
    }

const char* eBike_err_to_name(eBike_err_type_t);
void eBike_err_report(eBike_err_t err);

#endif
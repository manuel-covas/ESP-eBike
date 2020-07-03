#include <eBike_err.h>
#include <esp_gatts_api.h>
#include <sdkconfig.h>


void eBike_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    
}

eBike_err_t eBike_ble_init() {
    eBike_err_t eBike_err;

    EBIKE_HANDLE_ERROR(esp_ble_gatts_register_callback(eBike_gatts_callback), EBIKE_BLE_INIT_CALLBACK_REGISTER_FAIL, eBike_err);
    EBIKE_ERROR_OK(eBike_err); return eBike_err;
}
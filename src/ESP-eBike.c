#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <eBike_err.h>
#include <eBike_ble.h>
#include <eBike_nvs.h>


void app_main() {
    
    eBike_err_t err;
    
    err = eBike_nvs_init(); eBike_err_report(err);
    err = eBike_ble_init(); eBike_err_report(err);
    
    printf("eBike BLE init:\n\
            esp_err: %s\n\
            eBike_err_type: %s\n", esp_err_to_name(err.esp_err), eBike_err_to_name(err.eBike_err_type));
}
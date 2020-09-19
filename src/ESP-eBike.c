#include <string.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <util/eBike_util.h>
#include <eBike_err.h>
#include <eBike_nvs.h>
#include <eBike_gpio.h>
#include <eBike_ble.h>
#include <eBike_auth.h>
#include <eBike_bms.h>
#include <eBike_log.h>

void load_settings();


eBike_settings_t eBike_settings;

void app_main() {
    
    eBike_err_t err;
    
    err = eBike_nvs_init();  eBike_err_report(err);
    err = eBike_gpio_init(); eBike_err_report(err);
    err = eBike_ble_init();  eBike_err_report(err);
    err = eBike_auth_init(); eBike_err_report(err);
    err = eBike_bms_init();  eBike_err_report(err);
    
    load_settings();

    printf("[System] - Done booting.\n");
}


void load_settings() {

    char* message;
    eBike_err_t eBike_err = eBike_nvs_settings_get(&eBike_settings); if (eBike_err.eBike_err_type != EBIKE_OK) goto eBike_clean;
    

    eBike_log_add(message, strlen(message));
    free(message);
    

eBike_clean:
    asprintf(&message, "[System] - Failed to load settings from NVS: eBike_err: %s (%i) esp_err: %s (%i). Please re-configure them manually\n.", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err);
    eBike_log_add(message, strlen(message));
    free(message);
    return false;
}
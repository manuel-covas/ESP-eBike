#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <eBike_err.h>
#include <eBike_nvs.h>
#include <eBike_gpio.h>
#include <eBike_ble.h>


void app_main() {
    
    eBike_err_t err;
    
    err = eBike_nvs_init();  eBike_err_report(err);
    err = eBike_gpio_init(); eBike_err_report(err);
    err = eBike_ble_init();  eBike_err_report(err);
    
    printf("Done booting.\n");
}
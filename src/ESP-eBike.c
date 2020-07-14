#include <string.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <eBike_err.h>
#include <eBike_nvs.h>
#include <eBike_gpio.h>
#include <eBike_ble.h>
#include <eBike_log.h>
#include <mbedtls/sha256.h>


void logger_task(void* err_param) {
    
    char* message = calloc(100, 1);
    
    while (true) {
        memset(message, 0, 100);
        int length = sprintf(message, "[Logger] - Current time: %lli\n", esp_timer_get_time()/1000);
        eBike_log_add(message, length);
        printf("[Task] - Added to log.\n");
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}


void app_main() {
    
    eBike_err_t err;
    
    err = eBike_nvs_init();  eBike_err_report(err);
    err = eBike_gpio_init(); eBike_err_report(err);
    err = eBike_ble_init();  eBike_err_report(err);
    
    char* input = "Hello World";
    char* output = calloc(32, 1);
    eBike_log_add("[Auth] - Calculating hash of 'Hello World' with secret.\n", 56);

    mbedtls_sha256();

    xTaskCreate(logger_task, "Logger Task", 2000, NULL, tskIDLE_PRIORITY, NULL);
    printf("Done booting.\n");
}
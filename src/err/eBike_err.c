#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <eBike_err.h>

#define EBIKE_ERROR_BEEP_LONG_MS 500
#define EBIKE_ERROR_BEEP_SHORT_MS 200


const char* eBike_err_type_enum_names[] = {
    "EBIKE_NO_ERROR",
    "EBIKE_NVS_INIT_ERASE_FAIL",
    "EBIKE_NVS_INIT_FAIL",
    "EBIKE_NVS_INIT_OPEN_FAIL",
    "EBIKE_BLE_INIT_CALLBACK_REGISTER_FAIL"
};

const char* eBike_err_to_name(eBike_err_type_t err_type) {
    return eBike_err_type_enum_names[err_type];
}


void eBike_err_report_task(void* err_param) {
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level(GPIO_NUM_2, 0);

    eBike_err_t err = *(eBike_err_t*) err_param;
    
    uint8_t eBike_high_beeps = err.eBike_err_type/10;
    uint8_t eBike_low_beeps  = err.eBike_err_type%10;
    uint8_t esp_high_beeps   = err.esp_err/10;
    uint8_t esp_low_beeps    = err.esp_err%10;

    while (true) {

        printf("Reporter task error\nesp_err: %s eBike_err: %s\n", esp_err_to_name(err.esp_err), eBike_err_to_name(err.eBike_err_type));

        for (uint8_t i = eBike_high_beeps; i > 0; i--) {
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(EBIKE_ERROR_BEEP_LONG_MS / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_2, 0);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        for (uint8_t i = eBike_low_beeps; i > 0; i--) {
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(EBIKE_ERROR_BEEP_SHORT_MS / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_2, 0);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        vTaskDelay(1500 / portTICK_PERIOD_MS);

        for (uint8_t i = esp_high_beeps; i > 0; i--) {
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(EBIKE_ERROR_BEEP_LONG_MS / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_2, 0);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        for (uint8_t i = esp_low_beeps; i > 0; i--) {
            gpio_set_level(GPIO_NUM_2, 1);
            vTaskDelay(EBIKE_ERROR_BEEP_SHORT_MS / portTICK_PERIOD_MS);
            gpio_set_level(GPIO_NUM_2, 0);
            vTaskDelay(200 / portTICK_PERIOD_MS);
        }

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void eBike_err_report(eBike_err_t err) {
    if (err.esp_err == ESP_OK) return;

    xTaskCreate(eBike_err_report_task, "Error Reporter", 2000, &err, tskIDLE_PRIORITY, NULL);
    vTaskDelete(NULL);  // Stop main task execution.
}
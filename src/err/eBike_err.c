#include <sdkconfig.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_gpio.h>

const char* eBike_err_type_enum_names[] = {
    "EBIKE_OK",
    "EBIKE_NVS_INIT_ERASE_FAIL",
    "EBIKE_NVS_INIT_FAIL",
    "EBIKE_NVS_INIT_OPEN_FAIL",
    "EBIKE_GPIO_INIT_CONFIG_FAIL",
    "EBIKE_GPIO_INIT_PWM_CONFIG_FAIL",
    "EBIKE_GPIO_PWM_TOGGLE_FAIL",
    "EBIKE_GPIO_PWM_SET_DUTY_FAIL",
    "EBIKE_ADC_INIT_CREATE_UNIT_FAIL",
    "EBIKE_ADC_INIT_CONFIG_CHANNEL_FAIL",
    "EBIKE_ADC_INIT_CREATE_CALIBRATION_FAIL",
    "EBIKE_ADC_INIT_CHECK_VREF_TYPE_FAIL",
    "EBIKE_ADC_ONESHOT_READ_FAIL",
    "EBIKE_ADC_RAW_TO_VOLTAGE_CONVERSION_FAIL",
    "EBIKE_BLE_INIT_OUTGOING_QUEUE_CREATE_FAIL",
    "EBIKE_BLE_INIT_OUTGOING_TASK_CREATE_FAIL",
    "EBIKE_BLE_INIT_CONTROLLER_INIT_FAIL",
    "EBIKE_BLE_INIT_ENABLE_CONTROLLER_FAIL",
    "EBIKE_BLE_INIT_BLUEDROID_INIT_FAIL",
    "EBIKE_BLE_INIT_BLUEDROID_ENABLE_FAIL",
    "EBIKE_BLE_INIT_GAP_CALLBACK_REGISTER_FAIL",
    "EBIKE_BLE_INIT_GATTS_CALLBACK_REGISTER_FAIL",
    "EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL",
    "EBIKE_BLE_INIT_SET_BT_NAME_FAIL",
    "EBIKE_BLE_INIT_SET_ADV_DATA_FAIL",
    "EBIKE_BLE_INIT_START_ADV_FAIL",
    "EBIKE_BLE_TX_NOT_CONNECTED",
    "EBIKE_BLE_TX_BAD_ARGUMENTS",
    "EBIKE_BLE_TX_MALLOC_FAIL",
    "EBIKE_BLE_TX_QUEUE_FAIL",
    "EBIKE_BLE_COMMAND_TOO_SHORT",
    "EBIKE_BLE_COMMAND_UNKNOWN",
    "EBIKE_BLE_AUTHED_COMMAND_UNKNOWN",
    "EBIKE_LOG_INIT_MALLOC_FAIL",
    "EBIKE_LOG_TX_MALLOC_FAIL",
    "EBIKE_AUTH_INIT_MALLOC_FAIL",
    "EBIKE_AUTH_INIT_PARSE_KEY_FAIL",
    "EBIKE_AUTHED_COMMAND_FAIL",
    "EBIKE_NVS_SETTINGS_GET_FAIL",
    "EBIKE_NVS_SETTINGS_PUT_FAIL",
    "EBIKE_NVS_SETTINGS_CRC_MISMATCH",
    "EBIKE_BMS_INIT_I2C_CONFIG_FAIL",
    "EBIKE_BMS_INIT_I2C_INSTALL_FAIL",
    "EBIKE_BMS_I2C_BUILD_COMMAND_FAIL",
    "EBIKE_BMS_I2C_COMMAND_FAIL",
    "EBIKE_BMS_I2C_CRC_MISMATCH"
};

const char* eBike_err_to_name(eBike_err_type_t err_type) {
    return eBike_err_type_enum_names[err_type];
}


void eBike_err_report_task(void* err_param) {

    eBike_err_t err = *(eBike_err_t*) err_param;
    
    uint8_t eBike_high_beeps = err.eBike_err_type/10;
    uint8_t eBike_low_beeps  = err.eBike_err_type%10;
    uint8_t esp_high_beeps   = err.esp_err/10;
    uint8_t esp_low_beeps    = err.esp_err%10;

    char* log_message = calloc(1000, 1);

    sprintf(log_message, "[System] - Error state: eBike_err: %s (%i) esp_err: %s (%i)\n", eBike_err_to_name(err.eBike_err_type), err.eBike_err_type, esp_err_to_name(err.esp_err), err.esp_err);
    eBike_log_add(log_message, strlen(log_message));
    
    while (true) {
        
        printf(log_message);

        for (uint8_t i = eBike_high_beeps; i > 0; i--) {
            eBike_gpio_toggle_fault(1);
            vTaskDelay(EBIKE_ERROR_BEEP_LONG_MS / portTICK_PERIOD_MS);
            eBike_gpio_toggle_fault(0);
            vTaskDelay(EBIKE_ERROR_BEEP_LONG_MS / portTICK_PERIOD_MS);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        for (uint8_t i = eBike_low_beeps; i > 0; i--) {
            eBike_gpio_toggle_fault(1);
            vTaskDelay(EBIKE_ERROR_BEEP_SHORT_MS / portTICK_PERIOD_MS);
            eBike_gpio_toggle_fault(0);
            vTaskDelay(EBIKE_ERROR_BEEP_SHORT_MS / portTICK_PERIOD_MS);
        }

        vTaskDelay(1500 / portTICK_PERIOD_MS);

        for (uint8_t i = esp_high_beeps; i > 0; i--) {
            eBike_gpio_toggle_fault(1);
            vTaskDelay(EBIKE_ERROR_BEEP_LONG_MS / portTICK_PERIOD_MS);
            eBike_gpio_toggle_fault(0);
            vTaskDelay(EBIKE_ERROR_BEEP_LONG_MS / portTICK_PERIOD_MS);
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);

        for (uint8_t i = esp_low_beeps; i > 0; i--) {
            eBike_gpio_toggle_fault(1);
            vTaskDelay(EBIKE_ERROR_BEEP_SHORT_MS / portTICK_PERIOD_MS);
            eBike_gpio_toggle_fault(0);
            vTaskDelay(EBIKE_ERROR_BEEP_SHORT_MS / portTICK_PERIOD_MS);
        }

        vTaskDelay(4000 / portTICK_PERIOD_MS);
    }
}

void eBike_err_report(eBike_err_t err) {
    if (err.esp_err == ESP_OK && err.eBike_err_type == EBIKE_OK) return;

    xTaskCreate(eBike_err_report_task, "Error Reporter", 2000, &err, tskIDLE_PRIORITY, NULL);
    vTaskDelete(NULL);  // Stop calling task's execution.
}


void eBike_beep_task(void* duration_param) {
    eBike_gpio_toggle_fault(1);
    vTaskDelay(*(uint32_t*) duration_param / portTICK_PERIOD_MS);
    eBike_gpio_toggle_fault(0);
    vTaskDelete(NULL);
}
void eBike_beep(uint32_t* duration_ms) {
    xTaskCreate(eBike_beep_task, "Beeper", 2000, duration_ms, tskIDLE_PRIORITY, NULL);
}
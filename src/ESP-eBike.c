#include <string.h>
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_err.h>
#include <nvs.h>
#include <util/eBike_util.h>
#include <eBike_err.h>
#include <eBike_nvs.h>
#include <eBike_gpio.h>
#include <eBike_adc.h>
#include <eBike_ble.h>
#include <eBike_ble_io.h>
#include <eBike_auth.h>
#include <eBike_bms.h>
#include <eBike_log.h>



bq76930_adc_characteristics_t bq76930_adc_characteristics;
eBike_settings_t eBike_settings;

void app_main() {
    
    eBike_err_t eBike_err = {
        .eBike_err_type = EBIKE_OK,
        .esp_err = ESP_OK
    };
    char* log_message;

    eBike_err = eBike_nvs_init();  eBike_err_report(eBike_err);
    eBike_err = eBike_gpio_init(); eBike_err_report(eBike_err);
    eBike_err = eBike_adc_init();  eBike_err_report(eBike_err);
    eBike_err = eBike_ble_init();  eBike_err_report(eBike_err);
    eBike_err = eBike_auth_init(); eBike_err_report(eBike_err);
    eBike_err = eBike_bms_init();  eBike_err_report(eBike_err);
    
    bq76930_adc_characteristics = bq76930_get_adc_characteristics();

    // Get settings from NVS

    eBike_err = eBike_nvs_settings_get(&eBike_settings);
    
    if (eBike_err.esp_err == ESP_ERR_NVS_NOT_FOUND) {

        // No saved settings, populate with defaults.

        EBIKE_NVS_DEFAULT_SETTINGS(&eBike_settings, bq76930_adc_characteristics);
        eBike_err = eBike_nvs_settings_put(&eBike_settings);
        
        char* message = eBike_print_settings(eBike_settings, "[NVS] - No settings found! Saving default values:");
        eBike_log_add(message, strlen(message));
        free(message);

        // Crash on failure
        eBike_err_report(eBike_err);
        
    }else if (eBike_err.eBike_err_type != EBIKE_OK || eBike_err.esp_err != ESP_OK) {
        
        // Report some other error.

        asprintf(&log_message, "[System] - Failed to load settings from NVS: eBike_err: %s (%i) esp_err: %s (%i). Please re-configure them manually.\n",
                        eBike_err_to_name(eBike_err.eBike_err_type),
                        eBike_err.eBike_err_type,
                        esp_err_to_name(eBike_err.esp_err),
                        eBike_err.esp_err);
        eBike_log_add(log_message, strlen(log_message));
        free(log_message);
        eBike_err_report(eBike_err);
    }


    log_message = eBike_print_settings(eBike_settings, "[System] - Loaded settings form NVS:");
    eBike_log_add(log_message, strlen(log_message));
    free(log_message);
    

    // Configure BQ76930

    eBike_err = eBike_bms_config(eBike_settings);
    
    if (eBike_err.eBike_err_type != EBIKE_OK) {
        asprintf(&log_message, "[System] - Failed to configure BQ76930 with NVS settings: eBike_err: %s (%i) esp_err: %s (%i).\n",
                                eBike_err_to_name(eBike_err.eBike_err_type),
                                eBike_err.eBike_err_type,
                                esp_err_to_name(eBike_err.esp_err),
                                eBike_err.esp_err);
        eBike_log_add(log_message, strlen(log_message));
        free(log_message);
        eBike_err_report(eBike_err);
    }
    
    printf("[System] - Done booting.\n");


    int64_t current_time = 0;
    int64_t last_ble_update = 0;
    int64_t last_cell_voltages_read = 0;
    eBike_system_stats_t system_stats;

    eBike_response_t eBike_response = {
        .eBike_response = 0xFF,
        .eBike_err = {
            .eBike_err_type = EBIKE_OK,
            .esp_err = ESP_OK
        }
    };


    while (1) {
        current_time = esp_timer_get_time();
        
        eBike_adc_read_throttle(&system_stats.throttle_percentage);
        eBike_err = eBike_gpio_pwm_set_duty(system_stats.throttle_percentage);                          if (eBike_err.eBike_err_type != EBIKE_OK) break;
        
        eBike_err = bq76930_read_register(BQ76930_SYS_STAT,   (uint8_t*) &system_stats.sys_stat);       if (eBike_err.eBike_err_type != EBIKE_OK) break;
        eBike_err = bq76930_read_register(BQ76930_SYS_CTRL_1, (uint8_t*) &system_stats.sys_ctrl1);      if (eBike_err.eBike_err_type != EBIKE_OK) break;
        eBike_err = bq76930_read_register(BQ76930_SYS_CTRL_2, (uint8_t*) &system_stats.sys_ctrl2);      if (eBike_err.eBike_err_type != EBIKE_OK) break;
        eBike_err = bq76930_read_register(BQ76930_CELLBAL,    (uint8_t*) &system_stats.cell_balancing); if (eBike_err.eBike_err_type != EBIKE_OK) break;
        
        if (system_stats.sys_stat.coulomb_counter_ready) {
            eBike_err = eBike_bms_read_current(&system_stats.pack_current);                             if (eBike_err.eBike_err_type != EBIKE_OK) break;
        }

        if (current_time - last_cell_voltages_read > 250000) {
            last_cell_voltages_read = current_time;
            eBike_err = eBike_bms_read_cell_voltages(&system_stats.cell_voltages);                      if (eBike_err.eBike_err_type != EBIKE_OK) break;
            eBike_err = eBike_bms_read_pack_voltage(&system_stats.pack_voltage);                        if (eBike_err.eBike_err_type != EBIKE_OK) break;
        }

        if (system_stats_stream_enabled && (current_time - last_ble_update > CONFIG_SYSTEM_STATS_STREAM_MIN_PERIOD_MS * 1000)) {
            last_ble_update = current_time;
            eBike_queue_ble_message(&eBike_response, &system_stats, sizeof(eBike_system_stats_t), false);
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    eBike_err_report(eBike_err);
}
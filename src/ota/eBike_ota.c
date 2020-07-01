#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>
#include <esp_ota_ops.h>

void check_ota() {

    esp_err_t error;
    const esp_partition_t* current_partition = esp_ota_get_running_partition();
    esp_ota_img_states_t current_partition_ota_state;

    if (current_partition == NULL) {
        printf("[Error] - Couldn't get running partition.");
        vTaskDelete(NULL);
    }
    
    error = esp_ota_get_state_partition(current_partition, &current_partition_ota_state);

    if (error != ESP_OK) {
        if (error == ESP_ERR_NOT_SUPPORTED) {
            printf("[Error] Current partition is not an OTA partition.\n"); return;
        }

        printf("[Error] Error getting current partiton OTA state: %s\n", esp_err_to_name(error));
        vTaskDelete(NULL);
    }
    
    if (current_partition_ota_state == ESP_OTA_IMG_PENDING_VERIFY) {    //TODO: Test for nvs.
        
        error = esp_ota_mark_app_valid_cancel_rollback();
        
        if (error != ESP_OK) {
            printf("[Error] Error marking ota valid: %s\n", esp_err_to_name(error));
            vTaskDelete(NULL);
        }
    }
}
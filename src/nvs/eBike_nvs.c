#include <string.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_nvs.h>
#include <nvs_flash.h>
#include <nvs.h>

#define EBIKE_NVS_NAMESPACE "ESP-eBike"
#define EBIKE_NVS_SETTIGNS_KEY "eBike_settings"

nvs_handle_t eBike_nvs_handle;


eBike_err_t eBike_nvs_init() {
    
    eBike_err_t eBike_err;
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {  // Bad NVS partition, erase and retry.
        EBIKE_HANDLE_ERROR(nvs_flash_erase(), EBIKE_NVS_INIT_ERASE_FAIL, eBike_err);
        printf("[NVS] - Erased invalid NVS partition.");
        err = nvs_flash_init();
    }

    EBIKE_HANDLE_ERROR(err, EBIKE_NVS_INIT_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(nvs_open(EBIKE_NVS_NAMESPACE, NVS_READWRITE, &eBike_nvs_handle), EBIKE_NVS_INIT_OPEN_FAIL, eBike_err);

eBike_clean:
    return eBike_err;
}


eBike_err_t eBike_nvs_settings_get(eBike_settings_t* pointer) {

    eBike_err_t eBike_err;
    eBike_settings_t result;
    size_t length = sizeof(eBike_settings_t);

    EBIKE_HANDLE_ERROR(nvs_get_blob(eBike_nvs_handle, EBIKE_NVS_SETTIGNS_KEY, &result, &length), EBIKE_NVS_SETTINGS_GET_FAIL, eBike_err);

    uint32_t crc;
    crc32(&result, sizeof(eBike_settings_t) - 4, &crc);

    if (crc != result.crc32) {
        eBike_err.eBike_err_type = EBIKE_NVS_SETTINGS_CRC_MISMATCH;
        eBike_err.esp_err = ESP_OK;
    }else{
        memcpy(pointer, &result, sizeof(eBike_settings_t));
    }
    
eBike_clean:
    return eBike_err;
}
#include <eBike_err.h>
#include <nvs_flash.h>
#include <nvs.h>

#define EBIKE_NVS_NAMESPACE "eBike"

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
    
    return eBike_err;
}
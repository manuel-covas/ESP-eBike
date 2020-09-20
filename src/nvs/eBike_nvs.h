#ifndef EBIKE_NVS_H
#define EBIKE_NVS_H

#include <eBike_err.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct eBike_settings_t {
    bool bq76930_use_internal_thermistor;
    
    uint8_t bq76930_short_circuit_delay:2;
    uint8_t bq76930_short_circuit_threshold:3;
    bool bq76930_double_thresholds:1;
    uint8_t bq76930_overcurrent_threshold:4;
    uint8_t bq76930_overcurrent_delay:3;
    uint8_t bq76930_overvoltage_delay:2;
    uint8_t bq76930_undervoltage_delay:2;
    
    uint8_t bq76930_overvoltage_threshold;
    uint8_t bq76930_undervoltage_threshold;
    
    uint32_t crc32;

} eBike_settings_t;

eBike_err_t eBike_nvs_init();
eBike_err_t eBike_nvs_settings_get(eBike_settings_t* pointer);

#endif
#ifndef EBIKE_NVS_H
#define EBIKE_NVS_H

#include <eBike_err.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct __attribute__((__packed__)) eBike_settings_t {

    bool bq76930_use_internal_thermistor;
    
    uint8_t bq76930_short_circuit_delay;
    uint8_t bq76930_short_circuit_threshold;
    bool bq76930_double_thresholds;
    uint8_t bq76930_overcurrent_threshold;
    uint8_t bq76930_overcurrent_delay;
    uint8_t bq76930_overvoltage_delay;
    uint8_t bq76930_undervoltage_delay;
    
    uint8_t bq76930_overvoltage_threshold;
    uint8_t bq76930_undervoltage_threshold;
    
    uint32_t crc32;

} eBike_settings_t;

eBike_err_t eBike_nvs_init();
eBike_err_t eBike_nvs_settings_get(eBike_settings_t* pointer);
eBike_err_t eBike_nvs_settings_put(eBike_settings_t* pointer);


#endif
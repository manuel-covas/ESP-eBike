#ifndef EBIKE_NVS_H
#define EBIKE_NVS_H

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
    
    uint32_t crc32;  // CRC-32/MPEG-2 checksum of the previous bytes (LSB)

} eBike_settings_t;

#include <eBike_err.h>
#include <eBike_util.h>
#include <bq76930.h>

eBike_err_t eBike_nvs_init();
eBike_err_t eBike_nvs_settings_get(eBike_settings_t* pointer);
eBike_err_t eBike_nvs_settings_put(eBike_settings_t* pointer);

#define EBIKE_NVS_DEFAULT_SETTINGS(eBike_settings_pointer, bq76930_adc_characteristics)                                                        \
    (eBike_settings_pointer)->bq76930_use_internal_thermistor = true;                                                                          \
    (eBike_settings_pointer)->bq76930_short_circuit_delay = 0x00;                                                                              \
    (eBike_settings_pointer)->bq76930_short_circuit_threshold = 0x00;                                                                          \
    (eBike_settings_pointer)->bq76930_double_thresholds = false;                                                                               \
    (eBike_settings_pointer)->bq76930_overcurrent_threshold = 0x00;                                                                            \
    (eBike_settings_pointer)->bq76930_overcurrent_delay = 0x00;                                                                                \
    (eBike_settings_pointer)->bq76930_overvoltage_delay = 0x00;                                                                                \
    (eBike_settings_pointer)->bq76930_undervoltage_delay = 0x00;                                                                               \
    (eBike_settings_pointer)->bq76930_overvoltage_threshold = (BQ76930_ADC_OV_TRIP_REVERSE_TRANSFER_VOLTS(4.1, bq76930_adc_characteristics));  \
    (eBike_settings_pointer)->bq76930_undervoltage_threshold = 0xFF;                                                                           \
    (eBike_settings_pointer)->crc32 = xcrc32((uint8_t*) (eBike_settings_pointer), sizeof(eBike_settings_t) - sizeof((eBike_settings_pointer)->crc32));

#endif
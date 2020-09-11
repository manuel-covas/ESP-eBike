#ifndef EBIKE_NVS_H
#define EBIKE_NVS_H

#include <eBike_err.h>

typedef struct eBike_settings_t {
    bool bq76930_use_internal_thermistor;
} eBike_settings_t;

eBike_err_t eBike_nvs_init();


#endif
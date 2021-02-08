#ifndef EBIKE_BMS_H
#define EBIKE_BMS_H

#include <eBike_err.h>
#include <eBike_nvs.h>

typedef struct __attribute__((__packed__)) eBike_cell_voltages_t {
    double cell_1;
    double cell_2;
    double cell_3;
    double cell_4;
    double cell_5;
    double cell_6;
    double cell_7;
    double cell_8;
    double cell_9;
    double cell_10;
} eBike_cell_voltages_t;

eBike_err_t eBike_bms_init();
eBike_err_t eBike_bms_config(eBike_settings_t eBike_settings);
eBike_err_t eBike_bms_read_cell_voltages(eBike_cell_voltages_t* eBike_cell_voltages);
eBike_err_t eBike_bms_read_pack_voltage(double* pack_voltage);
eBike_err_t eBike_bms_read_current(double* pack_current);

#endif
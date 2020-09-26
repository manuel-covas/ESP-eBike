#include <malloc.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_nvs.h>
#include <bq76930.h>
#include <sdkconfig.h>


bool bms_configured = false;


eBike_err_t eBike_bms_init() {

    eBike_err_t eBike_err;

    printf("[BMS] - Initializing...\n");
    eBike_err = bq76930_init(); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;


    // Checking if DEVICE_XREADY is set.
    bq76930_sys_stat_t sys_stat;
    eBike_err = bq76930_read_register(BQ76930_SYS_STAT, (uint8_t*) &sys_stat); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    if (sys_stat.device_xready) {
        bq76930_sys_stat_t sys_stat_clear = {
            .device_xready = true
        };
        printf("[BMS] - BQ76930 SYS_STAT has DEVICE_XREADY set. Clearing...\n");
        eBike_err = bq76930_write_register(BQ76930_SYS_STAT, (uint8_t*) &sys_stat_clear); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    }

    // Turn off charge and discharge MOSFETs.
    bq76930_sys_ctrl_2_t sys_ctrl_2 = {
        .disable_delays = false,
        .coulomb_counter_enable = true,
        .discharge_on = false,
        .charge_on = false
    };
    printf("[BMS] - Turning charge and discharge MOSFETs...\n");
    eBike_err = bq76930_write_register(BQ76930_SYS_CTRL_2, (uint8_t*) &sys_ctrl_2); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;


    // Ensure cell balancing is not active. 
    bq76930_cellbal_t cellbal;
    printf("[BMS] - Stopping cell balancing...\n");
    eBike_err = bq76930_write_register(BQ76930_CELLBAL, (uint8_t*) &cellbal); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;


    // Setting coulomb counter config value.
    bq76930_cc_cfg_t cc_config = {
        .coulomb_counter_config = 0x19
    };
    printf("[BMS] - Configuring coulomb counter...\n");
    eBike_err = bq76930_write_register(BQ76930_CC_CFG, (uint8_t*) &cc_config); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    return eBike_err;
}

eBike_err_t eBike_bms_config(eBike_settings_t eBike_settings) {

    printf("[BMS] - Writing settings to BQ76930...\n");
    eBike_err_t eBike_err;


    bq76930_sys_ctrl_1_t sys_ctrl_1 = {
        .adc_enable = true,
        .use_external_temp = !eBike_settings.bq76930_use_internal_thermistor
    };
    printf(eBike_settings.bq76930_use_internal_thermistor ? "[BMS] - Using internal die temperature.\n" : "[BMS] - Using external thermistors.\n");
    eBike_err = bq76930_write_register(BQ76930_SYS_CTRL_1, (uint8_t*) &sys_ctrl_1); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;


    bq76930_protect_t protect = {
        .short_circuit_threshold = eBike_settings.bq76930_short_circuit_threshold,
        .short_circuit_delay = eBike_settings.bq76930_short_circuit_delay,
        .double_thresholds = eBike_settings.bq76930_double_thresholds,
        .overcurrent_threshold = eBike_settings.bq76930_overcurrent_threshold,
        .overcurrent_delay = eBike_settings.bq76930_overcurrent_delay,
        .overvoltage_delay = eBike_settings.bq76930_overvoltage_delay,
        .undervoltage_delay = eBike_settings.bq76930_undervoltage_delay,
    };
    printf("[BMS] - Writing PROTECT settings...\n");
    eBike_err = bq76930_write_register(BQ76930_PROTECT, (uint8_t*) &protect); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;


    bq76930_ov_uv_trip_t overvoltage_undervoltage_thresholds = {
        .overvoltage_threshold = eBike_settings.bq76930_overvoltage_threshold,
        .undervoltage_threshold = eBike_settings.bq76930_undervoltage_threshold
    };
    printf("[BMS] - Writing overvoltage and undervoltage thresholds...\n");
    eBike_err = bq76930_write_register(BQ76930_OV_UV_TRIP, (uint8_t*) &overvoltage_undervoltage_thresholds); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    if (eBike_err.eBike_err_type == EBIKE_OK)
        bms_configured = true;

    return eBike_err;
}
#include <string.h>
#include <eBike_bms.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <bq76930.h>
#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


bq76930_adc_characteristics_t bq76930_adc_characteristics;

eBike_err_t eBike_bms_init() {

    eBike_err_t eBike_err;

    printf("[BMS] - Initializing...\n");
    eBike_err = bq76930_init(); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    bq76930_adc_characteristics = bq76930_get_adc_characteristics();

    // Setting coulomb counter config value.
    bq76930_cc_cfg_t cc_config = {
        .coulomb_counter_config = 0x19
    };
    printf("[BMS] - Configuring coulomb counter...\n");
    eBike_err = bq76930_write_register(BQ76930_CC_CFG, (uint8_t*) &cc_config);

    // Checking if DEVICE_XREADY is set.
    bq76930_sys_stat_t sys_stat;
    eBike_err = bq76930_read_register(BQ76930_SYS_STAT, (uint8_t*) &sys_stat); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    if (sys_stat.device_xready) {
        bq76930_sys_stat_t sys_stat_clear = {
            .device_xready = true
        };
        printf("[BMS] - BQ76930 SYS_STAT has DEVICE_XREADY set. Waiting...\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("[BMS] - Clearing...\n");
        eBike_err = bq76930_write_register(BQ76930_SYS_STAT, (uint8_t*) &sys_stat_clear); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    }

    // Turn off charge and discharge MOSFETs.
    bq76930_sys_ctrl_2_t sys_ctrl_2 = {
        .disable_delays = false,
        .coulomb_counter_enable = true,
        .discharge_on = false,
        .charge_on = false
    };
    printf("[BMS] - Enabling continuous coulomb counter and turning off charge and discharge MOSFETs...\n");
    eBike_err = bq76930_write_register(BQ76930_SYS_CTRL_2, (uint8_t*) &sys_ctrl_2); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    // Ensure cell balancing is not active. 
    bq76930_cellbal_t cellbal;
    printf("[BMS] - Stopping cell balancing...\n");
    eBike_err = bq76930_write_register(BQ76930_CELLBAL, (uint8_t*) &cellbal); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

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
        .double_thresholds = eBike_settings.bq76930_double_thresholds ? true : false,
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

    return eBike_err;
}


eBike_err_t eBike_bms_read_cell_voltages(eBike_cell_voltages_t* eBike_cell_voltages) {

    eBike_cell_voltages_t result;
    bq76930_cell_voltages_t bq76930_cell_voltages;
    
    eBike_err_t eBike_err = bq76930_read_register(BQ76930_CELL_VOLTAGES, (uint8_t*) &bq76930_cell_voltages);

    if (eBike_err.eBike_err_type != EBIKE_OK)
        goto eBike_clean;

    result.cell_1  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC1_HI)  << 8) | bq76930_cell_voltages.VC1_LO),  bq76930_adc_characteristics);
    result.cell_2  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC2_HI)  << 8) | bq76930_cell_voltages.VC2_LO),  bq76930_adc_characteristics);
    result.cell_3  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC3_HI)  << 8) | bq76930_cell_voltages.VC3_LO),  bq76930_adc_characteristics);
    result.cell_4  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC4_HI)  << 8) | bq76930_cell_voltages.VC4_LO),  bq76930_adc_characteristics);
    result.cell_5  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC5_HI)  << 8) | bq76930_cell_voltages.VC5_LO),  bq76930_adc_characteristics);
    result.cell_6  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC6_HI)  << 8) | bq76930_cell_voltages.VC6_LO),  bq76930_adc_characteristics);
    result.cell_7  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC7_HI)  << 8) | bq76930_cell_voltages.VC7_LO),  bq76930_adc_characteristics);
    result.cell_8  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC8_HI)  << 8) | bq76930_cell_voltages.VC8_LO),  bq76930_adc_characteristics);
    result.cell_9  = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC9_HI)  << 8) | bq76930_cell_voltages.VC9_LO),  bq76930_adc_characteristics);
    result.cell_10 = BQ76930_ADC_TRANSFER_VOLTS((((0x3F & bq76930_cell_voltages.VC10_HI) << 8) | bq76930_cell_voltages.VC10_LO), bq76930_adc_characteristics);

    memcpy(eBike_cell_voltages, &result, sizeof(eBike_cell_voltages_t));
    
eBike_clean:
    return eBike_err;
}
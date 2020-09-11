#include <malloc.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_nvs.h>
#include <bq76930.h>
#include <sdkconfig.h>


eBike_err_t eBike_bms_init() {

    eBike_settings_t eBike_settings = {
        .bq76930_use_internal_thermistor = true
    };

    printf("[BMS] - Initializing...\n");

    eBike_err_t eBike_err;
    bq76930_sys_stat_t sys_stat;

    eBike_err = bq76930_init();                                                if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    eBike_err = bq76930_read_register(BQ76930_SYS_STAT, (uint8_t*) &sys_stat); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    
    if (sys_stat.device_xready) {

        bq76930_sys_stat_t sys_stat_clear = {
            .device_xready = true
        };

        printf("[BMS] - BQ76930 SYS_STAT has DEVICE_XREADY set. Clearing...\n");
        eBike_err = bq76930_write_register(BQ76930_SYS_STAT, (uint8_t*) &sys_stat_clear); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    }


    bq76930_cellbal_t cellbal;
    printf("[BMS] - Clearing balancing...\n");
    eBike_err = bq76930_write_register(BQ76930_CELLBAL, (uint8_t*) &cellbal); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    

    bq76930_sys_ctrl_t sys_ctrl = {
        .adc_enable = true,
        .use_external_temp = !eBike_settings.bq76930_use_internal_thermistor,
        .disable_delays = false,
        .coulomb_counter_enable = true,
        .discharge_on = false,
        .charge_on = false
    };
    printf(CONFIG_BQ76930_INTERNAL_TEMPERATURE ? "[BMS] - Using internal die temperature.\n" : "[BMS] - Using external thermistors.\n");
    eBike_err = bq76930_write_register(BQ76930_SYS_CTRL, (uint8_t*) &sys_ctrl); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;


    return eBike_err;
}
#include <malloc.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <bq76930.h>
#include <sdkconfig.h>


eBike_err_t eBike_bms_init() {

    printf("[BMS] - Initializing...\n");

    eBike_err_t eBike_err;
    bq76930_sys_stat_t sys_stat;

    eBike_err = bq76930_init();                   if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    eBike_err = bq76930_read_sys_stat(&sys_stat); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    
    if (sys_stat.device_xready) {
        bq76930_sys_stat_t sys_stat_clear = {
            .device_xready = sys_stat.device_xready
        };

        printf("[BMS] - BQ76930 SYS_STAT has DEVICE_XREADY set. Clearing...\n");
        eBike_err = bq76930_write_sys_stat(sys_stat_clear); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    }

    bq76930_sys_ctrl1_t sys_ctrl1 = {
        .adc_enable = true,
        .use_external_temp = !CONFIG_BQ76930_INTERNAL_TEMPERATURE
    };
    printf(CONFIG_BQ76930_INTERNAL_TEMPERATURE ? "[BMS] - Using internal die temperature.\n" : "[BMS] - Using external thermistors.\n");
    eBike_err = bq76930_write_sys_ctrl1(sys_ctrl1); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    
    bq76930_sys_ctrl2_t sys_ctrl2 = {
        .coulomb_counter_enable = true,
        .discharge_on = false,
        .charge_on = false
    };
    eBike_err = bq76930_write_sys_ctrl2(sys_ctrl2); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    


    return eBike_err;
}
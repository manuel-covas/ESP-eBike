#include <malloc.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <bq76930.h>
#include <sdkconfig.h>


eBike_err_t eBike_bms_init() {

    printf("[BMS] - Initializing...");

    eBike_err_t eBike_err;
    bq76930_sys_stat_t sys_stat;

    eBike_err = bq76930_init();                   if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    eBike_err = bq76930_read_sys_stat(&sys_stat); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    
    bq76930_sys_stat_t sys_stat = bq76930_parse_sys_stat(sys_stat_response);

    if (sys_stat.device_xready)
    {
        bq76930_sys_stat_t sys_stat_clear = {
            .device_xready = sys_stat.device_xready
        };

        printf("[BMS] - BQ76930 had DEVICE_XREADY set. Bit cleared.")
    }

    return eBike_err;
}
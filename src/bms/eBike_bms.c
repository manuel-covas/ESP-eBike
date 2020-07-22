#include <malloc.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <bq76930.h>
#include <sdkconfig.h>


eBike_err_t eBike_bms_init() {
    eBike_err_t eBike_err;
    uint8_t* sys_stat_response = calloc(2, 1);

    eBike_err = bq76930_init(); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    eBike_err = bq76930_read_register(BQ76930_SYS_STAT, sys_stat_response, 2); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;

    char* log_message = calloc(100, 1);
    int length = sprintf(log_message, "[BMS] - sys_stat: %02X crc: %02X\n", *sys_stat_response, *(sys_stat_response+1));

    eBike_log_add(log_message, length);
    free(log_message);
    return eBike_err;
}
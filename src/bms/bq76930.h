#ifndef BQ76930_H
#define BQ76930_H

#include <eBike_err.h>

#define BQ76930_I2C_RETRIES 4


typedef enum {
    BQ76930_SYS_STAT,
    BQ76930_CELLBAL1,
    BQ76930_CELLBAL2,
    BQ76930_SYS_CTRL1 = 4,
    BQ76930_SYS_CTRL2
} bq76930_register_t;


typedef struct bq76930_sys_stat_t {
    bool coulomb_counter_ready;
    bool device_xready;
    bool override_alert;
    bool undervoltage;
    bool overvoltage;
    bool shortcircuit_discharge;
    bool overcurrent_discharge;
} bq76930_sys_stat_t;

eBike_err_t bq76930_init();

eBike_err_t bq76930_read_sys_stat(bq76930_sys_stat_t* sys_stat);

eBike_err_t bq76930_read_register(bq76930_register_t register_address, uint8_t* buffer, size_t length);

uint8_t crc8(uint8_t* ptr, uint8_t len);

#endif
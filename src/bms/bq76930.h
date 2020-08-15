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

typedef struct bq76930_sys_ctrl1_t {
    bool load_present;
    bool adc_enable;
    bool use_external_temp;
    bool shutdown_a;
    bool shutdown_b;
} bq76930_sys_ctrl1_t;

typedef struct bq76930_sys_ctrl2_t {
    bool disable_delays;
    bool coulomb_counter_enable;
    bool coulomb_counter_oneshot;
    bool discharge_on;
    bool charge_on;
} bq76930_sys_ctrl2_t;


eBike_err_t bq76930_init();

eBike_err_t bq76930_read_sys_stat(bq76930_sys_stat_t* sys_stat_ptr);
eBike_err_t bq76930_write_sys_stat(bq76930_sys_stat_t sys_stat);

eBike_err_t bq76930_read_sys_ctrl1(bq76930_sys_ctrl1_t* sys_ctrl1_ptr);
eBike_err_t bq76930_write_sys_ctrl1(bq76930_sys_ctrl1_t sys_ctrl1);

eBike_err_t bq76930_read_sys_ctrl2(bq76930_sys_ctrl2_t* sys_ctrl2_ptr);
eBike_err_t bq76930_write_sys_ctrl2(bq76930_sys_ctrl2_t sys_ctrl2);

uint8_t crc8(uint8_t* ptr, uint8_t len);

#endif
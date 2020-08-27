#ifndef BQ76930_H
#define BQ76930_H

#include <eBike_err.h>

#define BQ76930_I2C_RETRIES 4


typedef enum {
    BQ76930_SYS_STAT,
    BQ76930_CELLBAL,
    BQ76930_SYS_CTRL = 4,
    BQ76930_PROTECT = 6
} bq76930_register_t;


typedef struct bq76930_sys_stat_t {
    bool overcurrent_discharge:1;
    bool shortcircuit_discharge:1;
    bool overvoltage:1;
    bool undervoltage:1;
    bool override_alert:1;
    bool device_xready:1;
    bool reserved:1;
    bool coulomb_counter_ready:1;
} bq76930_sys_stat_t;

typedef struct bq76930_cellbal_t {
    bool cell_1:1;
    bool cell_2:1;
    bool cell_3:1;
    bool cell_4:1;
    bool cell_5:1;
    uint8_t padding:3;
    bool cell_6:1;
    bool cell_7:1;
    bool cell_8:1;
    bool cell_9:1;
    bool cell_10:1;
} bq76930_cellbal_t;

typedef struct bq76930_sys_ctrl_t {
    bool shutdown_b:1;
    bool shutdown_a:1;
    bool reserved:1;
    bool use_external_temp:1;
    bool adc_enable:1;
    uint8_t padding:2;
    bool load_present:1;
    bool charge_on:1;
    bool discharge_on:1;
    uint8_t reserved:3;
    bool coulomb_counter_oneshot:1;
    bool coulomb_counter_enable:1;
    bool disable_delays:1;
} bq76930_sys_ctrl_t;

typedef struct bq76930_protect_t {
    uint8_t short_circuit_threshold:3;
    uint8_t short_circuit_delay:2;
    uint8_t reserved:2;
    bool double_thresholds:1;
} bq76930_protect_t;


eBike_err_t bq76930_init();

eBike_err_t bq76930_read_sys_stat(bq76930_sys_stat_t* sys_stat_ptr);
eBike_err_t bq76930_write_sys_stat(bq76930_sys_stat_t sys_stat);

eBike_err_t bq76930_read_sys_ctrl1(bq76930_sys_ctrl1_t* sys_ctrl1_ptr);
eBike_err_t bq76930_write_sys_ctrl1(bq76930_sys_ctrl1_t sys_ctrl1);

eBike_err_t bq76930_read_sys_ctrl2(bq76930_sys_ctrl2_t* sys_ctrl2_ptr);
eBike_err_t bq76930_write_sys_ctrl2(bq76930_sys_ctrl2_t sys_ctrl2);

uint8_t crc8(uint8_t* ptr, uint8_t len);

#endif
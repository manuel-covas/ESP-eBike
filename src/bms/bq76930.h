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
    bool reserved_1:1;
    bool coulomb_counter_ready:1;
} bq76930_sys_stat_t;

typedef struct bq76930_cellbal_t {
    bool cell_1:1;
    bool cell_2:1;
    bool cell_3:1;
    bool cell_4:1;
    bool cell_5:1;
    uint8_t reserved_1:3;
    bool cell_6:1;
    bool cell_7:1;
    bool cell_8:1;
    bool cell_9:1;
    bool cell_10:1;
} bq76930_cellbal_t;

typedef struct bq76930_sys_ctrl_t {
    bool shutdown_b:1;
    bool shutdown_a:1;
    bool reserved_1:1;
    bool use_external_temp:1;
    bool adc_enable:1;
    uint8_t reserved_2:2;
    bool load_present:1;
    bool charge_on:1;
    bool discharge_on:1;
    uint8_t reserved_3:3;
    bool coulomb_counter_oneshot:1;
    bool coulomb_counter_enable:1;
    bool disable_delays:1;
} bq76930_sys_ctrl_t;

typedef struct bq76930_protect_t {
    uint8_t short_circuit_threshold:3;
    uint8_t short_circuit_delay:2;
    uint8_t reserved_1:2;
    bool double_thresholds:1;
    uint8_t overcurrent_threshold:4;
    uint8_t overcurrent_delay:3;
    uint8_t reserved_2:5;
    uint8_t overvoltage_delay:2;
    uint8_t undervoltage_delay:2;
} bq76930_protect_t;

typedef struct bq76930_ov_trip_t {
    uint8_t overvoltage_threshold;
} bq76930_ov_trip_t;

typedef struct bq76930_uv_trip_t {
    uint8_t undervoltage_threshold;
} bq76930_uv_trip_t;


// Must be set to 0x19
typedef struct bq76930_cc_cfg_t {
    uint8_t coulomb_counter_config:6;
} bq76930_uv_trip_t;

typedef struct bq76930_cell_voltages_t {
    uint8_t VC1_HI:6;
    uint8_t reserved_1:2;
    uint8_t VC1_LO;
    uint8_t VC2_HI:6;
    uint8_t reserved_2:2;
    uint8_t VC2_LO;
    uint8_t VC3_HI:6;
    uint8_t reserved_3:2;
    uint8_t VC3_LO;
    uint8_t VC4_HI:6;
    uint8_t reserved_4:2;
    uint8_t VC4_LO;
    uint8_t VC5_HI:6;
    uint8_t reserved_5:2;
    uint8_t VC5_LO;
    uint8_t VC6_HI:6;
    uint8_t reserved_6:2;
    uint8_t VC6_LO;
    uint8_t VC7_HI:6;
    uint8_t reserved_7:2;
    uint8_t VC7_LO;
    uint8_t VC8_HI:6;
    uint8_t reserved_8:2;
    uint8_t VC8_LO;
    uint8_t VC9_HI:6;
    uint8_t reserved_9:2;
    uint8_t VC9_LO;
    uint8_t VC10_HI:6;
    uint8_t reserved_10:2;
    uint8_t VC10_LO;
} bq76930_cell_voltages_t;

typedef struct bq76930_bat_voltage_t {
    uint16_t battery_voltge;
} bq76930_bat_voltage_t;

typedef struct bq76930_ts1_t {
    uint8_t TS1_HI:6;
    uint8_t TS1_LO;
} bq76930_ts1_t;

typedef struct bq76930_ts2_t {
    uint8_t TS2_HI:6;
    uint8_t TS2_LO;
} bq76930_ts2_t;

typedef struct bq76930_coulomb_counter_t {
    uint16_t cc_reading;
} bq76930_coulomb_counter_t;



eBike_err_t bq76930_init();

uint8_t crc8(uint8_t* ptr, uint8_t len);

#endif

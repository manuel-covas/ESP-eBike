#ifndef BQ76930_H
#define BQ76930_H

#include <eBike_err.h>
#include <eBike_nvs.h>


#define BQ76930_I2C_RETRIES 4


typedef enum {
    BQ76930_SYS_STAT = 0x00,
    BQ76930_CELLBAL = 0x01,
    BQ76930_SYS_CTRL_1 = 0x04,
    BQ76930_SYS_CTRL_2 = 0x05,
    BQ76930_PROTECT = 0x06,
    BQ76930_OV_UV_TRIP = 0x09,
    BQ76930_CC_CFG = 0x0B,
    BQ76930_CELL_VOLTAGES = 0x0C,
    BQ76930_BAT_VOLTAGE = 0x2A,
    BQ76930_TS1 = 0x2C,
    BQ76930_TS2 = 0x2E,
    BQ76930_COULOMB_COUNTER = 0x32,
    BQ76930_ADC_GAIN_1 = 0x50,
    BQ76930_ADC_OFFSET = 0x51,
    BQ76930_ADC_GAIN_2 = 0x59
} bq76930_register_t;


const char* bq76930_register_to_name(bq76930_register_t register_address);
uint8_t bq76930_sizeof_register(bq76930_register_t register_address);


typedef struct __attribute__((__packed__)) bq76930_sys_stat_t {
    bool overcurrent_discharge:1;
    bool shortcircuit_discharge:1;
    bool overvoltage:1;
    bool undervoltage:1;
    bool override_alert:1;
    bool device_xready:1;
    bool reserved_1:1;
    bool coulomb_counter_ready:1;
} bq76930_sys_stat_t;

typedef struct __attribute__((__packed__)) bq76930_cellbal_t {
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

typedef struct __attribute__((__packed__)) bq76930_sys_ctrl_1_t {
    bool shutdown_b:1;
    bool shutdown_a:1;
    bool reserved_1:1;
    bool use_external_temp:1;
    bool adc_enable:1;
    uint8_t reserved_2:2;
    bool load_present:1;
} bq76930_sys_ctrl_1_t;

typedef struct __attribute__((__packed__)) bq76930_sys_ctrl_2_t {
    bool charge_on:1;
    bool discharge_on:1;
    uint8_t reserved_3:3;
    bool coulomb_counter_oneshot:1;
    bool coulomb_counter_enable:1;
    bool disable_delays:1;
} bq76930_sys_ctrl_2_t;

typedef struct __attribute__((__packed__)) bq76930_protect_t {
    uint8_t short_circuit_threshold:3;
    uint8_t short_circuit_delay:2;
    uint8_t reserved_1:2;
    bool double_thresholds:1;
    uint8_t overcurrent_threshold:4;
    uint8_t overcurrent_delay:3;
    uint8_t reserved_2:1;
    uint8_t reserved_3:4;
    uint8_t overvoltage_delay:2;
    uint8_t undervoltage_delay:2;
} bq76930_protect_t;

typedef struct __attribute__((__packed__)) bq76930_ov_uv_trip_t {
    uint8_t overvoltage_threshold;
    uint8_t undervoltage_threshold;
} bq76930_ov_uv_trip_t;

// Must be set to 0x19
typedef struct __attribute__((__packed__)) bq76930_cc_cfg_t {
    uint8_t coulomb_counter_config:6;
} bq76930_cc_cfg_t;

typedef struct __attribute__((__packed__)) bq76930_cell_voltages_t {
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

typedef struct __attribute__((__packed__)) bq76930_bat_voltage_t {
    uint8_t BAT_HI:6;
    uint8_t BAT_LO;
} bq76930_bat_voltage_t;

typedef struct __attribute__((__packed__)) bq76930_ts1_t {
    uint8_t TS1_HI:6;
    uint8_t TS1_LO;
} bq76930_ts1_t;

typedef struct __attribute__((__packed__)) bq76930_ts2_t {
    uint8_t TS2_HI:6;
    uint8_t TS2_LO;
} bq76930_ts2_t;

typedef struct __attribute__((__packed__)) bq76930_coulomb_counter_t {
    uint8_t CC_HI:6;
    uint8_t CC_LO;
} bq76930_coulomb_counter_t;

typedef struct __attribute__((__packed__)) bq76930_adc_gain_1_t {
    uint8_t reserved_1:2;
    uint8_t adc_gain_1:2;
} bq76930_adc_gain_1_t;

typedef struct __attribute__((__packed__)) bq76930_adc_gain_2_t {
    uint8_t reserved_1:5;
    uint8_t adc_gain_2:3;
} bq76930_adc_gain_2_t;

typedef struct __attribute__((__packed__)) bq76930_adc_offset_t {
    int8_t adc_offset;
} bq76930_adc_offset_t;


typedef struct __attribute__((__packed__)) bq76930_adc_characteristics_t {
    double shunt_value;
    int adc_gain_microvolts;
    int adc_offset_microvolts;
} bq76930_adc_characteristics_t;


eBike_err_t bq76930_init();
bq76930_adc_characteristics_t bq76930_get_adc_characteristics();

eBike_err_t bq76930_read_register(bq76930_register_t register_address, uint8_t* pointer);
eBike_err_t bq76930_write_register(bq76930_register_t register_address, uint8_t* pointer);

double bq76930_settings_overcurrent_amps(eBike_settings_t eBike_settings);
int bq76930_settings_overcurrent_delay_ms(eBike_settings_t eBike_settings);

double bq76930_settings_shortcircuit_amps(eBike_settings_t eBike_settings);
int bq76930_settings_shortcircuit_delay_us(eBike_settings_t eBike_settings);

double bq76930_settings_underoltage_trip_volts(eBike_settings_t eBike_settings);
int bq76930_settings_undervoltage_delay_seconds(eBike_settings_t eBike_settings);

double bq76930_settings_overvoltage_trip_volts(eBike_settings_t eBike_settings);
int bq76930_settings_overvoltage_delay_seconds(eBike_settings_t eBike_settings);


#define BQ76930_ADC_TRANSFER_VOLTS(adc_value, bq76930_adc_characteristics) (((double)(adc_value * bq76930_adc_characteristics.adc_gain_microvolts + bq76930_adc_characteristics.adc_offset_microvolts)) / 1000000.0)
#define BQ76930_ADC_OV_TRIP_REVERSE_TRANSFER_VOLTS(overvoltage_trip, bq76930_adc_characteristics)  ((((int)(((overvoltage_trip  * 1000000 - bq76930_adc_characteristics.adc_offset_microvolts) / bq76930_adc_characteristics.adc_gain_microvolts) - 0x2008)) / 0x10) & 0xFF)
#define BQ76930_ADC_UV_TRIP_REVERSE_TRANSFER_VOLTS(undervoltage_trip, bq76930_adc_characteristics) ((((int)(((undervoltage_trip * 1000000 - bq76930_adc_characteristics.adc_offset_microvolts) / bq76930_adc_characteristics.adc_gain_microvolts) - 0x1000)) / 0x10) & 0xFF)

#endif
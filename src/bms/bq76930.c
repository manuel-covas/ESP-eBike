#include <stdint.h>
#include <string.h>
#include <math.h>
#include <driver/i2c.h>
#include <bq76930.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_util.h>


bq76930_adc_characteristics_t adc_characteristics {
    .shunt_value = CONFIG_CURRENT_SENSE_RESISTOR / 1000,
    .adc_gain_microvolts = 0,
    .adc_offset_microvolts = 0
};

uint8_t overcurrent_table[32] = {8, 11, 14, 17, 19, 22, 25, 28, 31, 33, 36, 39, 42, 44, 47, 50,
                                 17, 22, 28, 33, 39, 44, 50, 56, 61, 67, 72, 78, 83, 89, 94, 100};

uint8_t shortcircuit_table[16] = {22, 33, 44, 56, 67, 78, 89, 100,
                                  44, 67, 89, 111, 133, 155, 178, 200};


eBike_err_t bq76930_init() {
    eBike_err_t eBike_err;
    
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = CONFIG_I2C_SDA_GPIO,
        .scl_io_num = CONFIG_I2C_SCL_GPIO,
        .sda_pullup_en = true,
        .scl_pullup_en = true,
        .master = {
            .clk_speed = CONFIG_I2C_CLOCK_FREQUENCY
        }
    };
    
    EBIKE_HANDLE_ERROR(i2c_param_config(I2C_NUM_0, &i2c_config), EBIKE_BMS_INIT_I2C_CONFIG_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0), EBIKE_BMS_INIT_I2C_INSTALL_FAIL, eBike_err);
    
    // Read adc gain and offset.
    bq76930_adc_gain_1_t adc_gain_1;
    bq76930_adc_gain_2_t adc_gain_2;
    bq76930_adc_offset_t adc_offset;

    printf("[BMS] - Reading ADC gain and offset...\n");

    eBike_err = bq76930_read_register(BQ76930_ADC_GAIN_1, (uint8_t*) &adc_gain_1); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    eBike_err = bq76930_read_register(BQ76930_ADC_GAIN_2, (uint8_t*) &adc_gain_2); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    eBike_err = bq76930_read_register(BQ76930_ADC_OFFSET, (uint8_t*) &adc_offset); if (eBike_err.eBike_err_type != EBIKE_OK) return eBike_err;
    
    adc_characteristics.adc_gain_microvolts = 365 + ((adc_gain_1.adc_gain_1 << 3) | adc_gain_2.adc_gain_2);
    adc_characteristics.adc_offset_microvolts = adc_offset.adc_offset * 1000;
    
    printf("[BMS] - ADC characteristics:\n"
           "    ADC Gain: %i uV/LSB\n"
           "    ADC_OFFSET: %i mV\n\n", adc_characteristics.adc_gain_microvolts, adc_offset.adc_offset);

eBike_clean:
    return eBike_err;
}

bq76930_adc_characteristics_t bq76930_adc_characteristics() {
    return adc_characteristics;
}

eBike_err_t bq76930_read_bytes(bq76930_register_t register_address, uint8_t* buffer, size_t length) {
    eBike_err_t eBike_err;
    
    i2c_cmd_handle_t i2c_command;
    TickType_t ticks_to_wait = 100 / portTICK_PERIOD_MS;

    i2c_command = i2c_cmd_link_create();
    EBIKE_HANDLE_ERROR(i2c_master_start(i2c_command), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_write_byte(i2c_command, (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_WRITE, true), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_write_byte(i2c_command, register_address, true), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_stop(i2c_command), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_cmd_begin(I2C_NUM_0, i2c_command, ticks_to_wait), EBIKE_BMS_I2C_COMMAND_FAIL, eBike_err);

    i2c_command = i2c_cmd_link_create();
    EBIKE_HANDLE_ERROR(i2c_master_start(i2c_command), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_write_byte(i2c_command, (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ, true), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_read(i2c_command, buffer, length - 1, I2C_MASTER_ACK), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_read(i2c_command, buffer + length - 1, 1, I2C_MASTER_NACK), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_stop(i2c_command), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_cmd_begin(I2C_NUM_0, i2c_command, ticks_to_wait), EBIKE_BMS_I2C_COMMAND_FAIL, eBike_err);

eBike_clean:
    i2c_cmd_link_delete(i2c_command);
    return eBike_err;
}

eBike_err_t bq76930_write_bytes(bq76930_register_t register_address, uint8_t* buffer, size_t length) {
    eBike_err_t eBike_err;
    
    i2c_cmd_handle_t i2c_command;
    TickType_t ticks_to_wait = 100 / portTICK_PERIOD_MS;
    
    i2c_command = i2c_cmd_link_create();
    EBIKE_HANDLE_ERROR(i2c_master_start(i2c_command), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_write_byte(i2c_command, (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_WRITE, true), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_write_byte(i2c_command, register_address, true), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_write(i2c_command, buffer, length, true), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_stop(i2c_command), EBIKE_BMS_I2C_BUILD_COMMAND_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_master_cmd_begin(I2C_NUM_0, i2c_command, ticks_to_wait), EBIKE_BMS_I2C_COMMAND_FAIL, eBike_err);

eBike_clean:
    i2c_cmd_link_delete(i2c_command);
    return eBike_err;
}


eBike_err_t bq76930_read_register(bq76930_register_t register_address, uint8_t* pointer) {
    
    uint8_t length = bq76930_sizeof_register(register_address) * 2;
    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;

    uint8_t* response = (uint8_t*) malloc(length + 1);
    response[0] = address_byte;


    int attempt = 0;
    while (true) {
        memset(response + 1, 0, length);

        eBike_err_t eBike_err = bq76930_read_bytes(register_address, response + 1, length);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = (char*) calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C read %s failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", bq76930_register_to_name(register_address),
                                                          eBike_err_to_name(eBike_err.eBike_err_type),
                                                          eBike_err.eBike_err_type,
                                                          esp_err_to_name(eBike_err.esp_err),
                                                          eBike_err.esp_err,
                                                          attempt + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);

        }else{

            uint8_t expected_crc = crc8(response, 2);
            uint8_t received_crc = *(response + 2);

            if (expected_crc != received_crc) goto crc_fail;

            for (int i = 1; i < (length / 2); i++) {
                expected_crc = crc8(response + 1 + i*2, 1);
                received_crc = *(response + 2 + i*2);

                if (expected_crc != received_crc) goto crc_fail;
            }

            for (int i = 0; i < length; i++) {
                memset(pointer + i, *(response + i*2), 1);
            }
            goto loop_end;

crc_fail:
            eBike_err.eBike_err_type = EBIKE_BMS_I2C_CRC_MISMATCH;

            char* log_message = (char*) calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C read %s failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                    "        Expected CRC %02X, got %02X\n"
                                    "        Attempt: %i\n", bq76930_register_to_name(register_address),
                                                             eBike_err_to_name(eBike_err.eBike_err_type),
                                                             eBike_err.eBike_err_type,
                                                             esp_err_to_name(eBike_err.esp_err),
                                                             eBike_err.esp_err,
                                                             expected_crc,
                                                             received_crc,
                                                             attempt + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }
        
loop_end:
        if (++attempt > BQ76930_I2C_RETRIES) {
            free(response);
            return eBike_err;
        }
    }
}

eBike_err_t bq76930_write_register(bq76930_register_t register_address, uint8_t* pointer) {
    
    uint8_t length = bq76930_sizeof_register(register_address) * 2;
    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_WRITE;
    uint8_t* message = (uint8_t*) malloc(length + 2);

    message[0] = address_byte;
    message[1] = register_address;
    message[2] = *pointer;
    message[3] = crc8(message, 3);

    for (int i = 1; i < length / 2; i++) {
        memset(message + 2 + i*2, *(pointer + i), 1);
        memset(message + 3 + i*2, crc8(pointer + i, 1), 1);
    }
    

    int attempt = 0;
    while (true) {

        eBike_err_t eBike_err = bq76930_write_bytes(register_address, message + 2, length);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = (char*) calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C write %s failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", bq76930_register_to_name(register_address),
                                                          eBike_err_to_name(eBike_err.eBike_err_type),
                                                          eBike_err.eBike_err_type,
                                                          esp_err_to_name(eBike_err.esp_err),
                                                          eBike_err.esp_err,
                                                          attempt + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);

        }else{
            /*
            printf("[BMS] - I2C wrote ");
            for (int i = 0; i < length; i++) {
                printf("%02X", *(message + 2 + i));
            }
            printf(" to %s.\n", bq76930_register_to_name(register_address)); */

            attempt = BQ76930_I2C_RETRIES;
        }
        
        if (++attempt > BQ76930_I2C_RETRIES) {
            free(message);
            return eBike_err;
        }
    }
}


double bq76930_settings_overcurrent_amps(eBike_settings_t eBike_settings) {
    return (overcurrent_table[eBike_settings.bq76930_double_thresholds * 16 + eBike_settings.bq76930_overcurrent_threshold] / 1000) / adc_characteristics.shunt_value;
}

int bq76930_settings_overcurrent_delay_ms(eBike_settings_t eBike_settings) {
    uint8_t code = eBike_settings.bq76930_overcurrent_delay;
    return code < 1 ? 8 : 20 * (int) pow(2, code - 1);
}


double bq76930_settings_shortcircuit_amps(eBike_settings_t eBike_settings) {
    return (shortcircuit_table[eBike_settings.bq76930_double_thresholds * 8 + eBike_settings.bq76930_short_circuit_threshold] / 1000) / adc_characteristics.shunt_value;
}

int bq76930_settings_shortcircuit_delay_us(eBike_settings_t eBike_settings) {
    uint8_t code = eBike_settings.bq76930_short_circuit_delay;
    return code < 1 ? 70 : 100 * (int) pow(2, code - 1);
}


double bq76930_settings_underoltage_trip_volts(eBike_settings_t eBike_settings) {
    int adc_mapping = (0b1 << 12) | (eBike_settings.bq76930_undervoltage_threshold << 4);
    return (double) (adc_mapping * adc_characteristics.adc_gain_microvolts + adc_characteristics.adc_offset_microvolts) / 1000000;
}

int bq76930_settings_undervoltage_delay_seconds(eBike_settings_t eBike_settings) {
    uint8_t code = eBike_settings.bq76930_undervoltage_delay;
    return code < 1 ? 1 : 4 * (int) pow(2, code - 1);
}


double bq76930_settings_overvoltage_trip_volts(eBike_settings_t eBike_settings) {
    int adc_mapping = (0b10 << 12) | (eBike_settings.bq76930_overvoltage_threshold << 4) | 0b1000;
    return (double) (adc_mapping * adc_characteristics.adc_gain_microvolts + adc_characteristics.adc_offset_microvolts) / 1000000;
}

int bq76930_settings_overvoltage_delay_seconds(eBike_settings_t eBike_settings) {
    uint8_t code = eBike_settings.bq76930_overvoltage_delay;
    return 1 * (int) pow(2, code);
}


const char* bq76930_register_to_name(bq76930_register_t register_address) {
    switch (register_address) {

    case BQ76930_SYS_STAT:
        return "BQ76930_SYS_STAT";

    case BQ76930_CELLBAL:
        return "BQ76930_CELLBAL";

    case BQ76930_SYS_CTRL_1:
        return "BQ76930_SYS_CTRL_1";

    case BQ76930_SYS_CTRL_2:
        return "BQ76930_SYS_CTRL_2";

    case BQ76930_PROTECT:
        return "BQ76930_PROTECT";

    case BQ76930_OV_UV_TRIP:
        return "BQ76930_OV_UV_TRIP";

    case BQ76930_CC_CFG:
        return "BQ76930_CC_CFG";

    case BQ76930_CELL_VOLTAGES:
        return "BQ76930_CELL_VOLTAGES";

    case BQ76930_BAT_VOLTAGE:
        return "BQ76930_BAT_VOLTAGE";

    case BQ76930_TS1:
        return "BQ76930_TS1";

    case BQ76930_TS2:
        return "BQ76930_TS2";

    case BQ76930_COULOMB_COUNTER:
        return "BQ76930_COULOMB_COUNTER";

    case BQ76930_ADC_GAIN_1:
        return "BQ76930_ADC_GAIN_1";

    case BQ76930_ADC_OFFSET:
        return "BQ76930_ADC_OFFSET";

    case BQ76930_ADC_GAIN_2:
        return "BQ76930_ADC_GAIN_2";

    default:
        return "UNKNOWN_REGISTER";
    }
}

uint8_t bq76930_sizeof_register(bq76930_register_t register_address) {
    switch (register_address) {

    case BQ76930_SYS_STAT:
        return sizeof(bq76930_sys_stat_t);

    case BQ76930_CELLBAL:
        return sizeof(bq76930_cellbal_t);

    case BQ76930_SYS_CTRL_1:
        return sizeof(bq76930_sys_ctrl_1_t);

    case BQ76930_SYS_CTRL_2:
        return sizeof(bq76930_sys_ctrl_2_t);

    case BQ76930_PROTECT:
        return sizeof(bq76930_protect_t);

    case BQ76930_OV_UV_TRIP:
        return sizeof(bq76930_ov_uv_trip_t);

    case BQ76930_CC_CFG:
        return sizeof(bq76930_cc_cfg_t);

    case BQ76930_CELL_VOLTAGES:
        return sizeof(bq76930_cell_voltages_t);

    case BQ76930_BAT_VOLTAGE:
        return sizeof(bq76930_bat_voltage_t);

    case BQ76930_TS1:
        return sizeof(bq76930_ts1_t);

    case BQ76930_TS2:
        return sizeof(bq76930_ts2_t);

    case BQ76930_COULOMB_COUNTER:
        return sizeof(bq76930_coulomb_counter_t);

    case BQ76930_ADC_GAIN_1:
        return sizeof(bq76930_adc_gain_1_t);

    case BQ76930_ADC_OFFSET:
        return sizeof(bq76930_adc_offset_t);

    case BQ76930_ADC_GAIN_2:
        return sizeof(bq76930_adc_gain_2_t);

    default:
        return 0;
    }
}
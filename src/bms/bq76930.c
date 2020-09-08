#include <stdint.h>
#include <string.h>
#include <driver/i2c.h>
#include <bq76930.h>
#include <eBike_err.h>
#include <eBike_log.h>


eBike_err_t bq76930_init() {
    eBike_err_t eBike_err;
    
    i2c_config_t i2c_config = {
        .mode = I2C_MODE_MASTER,
        .scl_io_num = CONFIG_I2C_SCL_GPIO,
        .sda_io_num = CONFIG_I2C_SDA_GPIO,
        .scl_pullup_en = true,
        .sda_pullup_en = true,
        .master = {
            .clk_speed = CONFIG_I2C_CLOCK_FREQUENCY
        }
    };
    
    EBIKE_HANDLE_ERROR(i2c_param_config(I2C_NUM_0, &i2c_config), EBIKE_BMS_INIT_I2C_CONFIG_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, 0), EBIKE_BMS_INIT_I2C_INSTALL_FAIL, eBike_err);
    
eBike_clean:
    return eBike_err;
}



/*
void* bq76930_parse_register(bq76930_register_t register_address, uint8_t byte) {
    switch (register_address)
    {
        case BQ76930_SYS_STAT:
            bq76930_sys_stat_t result = {
                .coulomb_counter_ready =  ((1 << 7) & byte) > 0,
                .device_xready =          ((1 << 5) & byte) > 0,
                .override_alert =         ((1 << 4) & byte) > 0,
                .undervoltage =           ((1 << 3) & byte) > 0,
                .overvoltage =            ((1 << 2) & byte) > 0,
                .shortcircuit_discharge = ((1 << 1) & byte) > 0,
                .overcurrent_discharge =  ((1 << 0) & byte) > 0
            };
            return &result;
        break;

        case BQ76930_CELLBAL:
            
        break;

    default:
        break;
    }
} */


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



eBike_err_t bq76930_read_register(bq76930_register_t register_address, void* pointer, bq76930_sys_stat_t* sys_stat_ptr) {
    
    uint8_t length = bq76930_sizeof_register(register_address) * 2;
    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;

    uint8_t* response = (uint8_t*) malloc(length + 1);
    response[0] = address_byte;


    int i = 0;
    while (true) {
        eBike_err_t eBike_err;

        memset(response + 1, 0, length);
        eBike_err = bq76930_read_bytes(register_address, response + 1, length);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = (char*) calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C read %s failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", bq76930_register_to_name(register_address),
                                                          eBike_err_to_name(eBike_err.eBike_err_type),
                                                          eBike_err.eBike_err_type,
                                                          esp_err_to_name(eBike_err.esp_err),
                                                          eBike_err.esp_err,
                                                          i + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }else{

            uint8_t crc_bytes[] = {address_byte, *response};
            uint8_t crc = crc8(crc_bytes, 2);

            if (crc != *(response + 1)) {
                eBike_err.eBike_err_type = EBIKE_BMS_I2C_CRC_MISMATCH;

                char* log_message = calloc(1000, 1);
                sprintf(log_message, "[BMS] - I2C read SYS_STAT failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                     "        Expected CRC %02X, got %02X\n"
                                     "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, crc, *(response + 1), i + 1);
                eBike_log_add(log_message, strlen(log_message));
                free(log_message);
            }else{
                bq76930_sys_stat_t sys_stat = bq76930_parse_sys_stat(*response);
                memcpy(sys_stat_ptr, &sys_stat, sizeof(bq76930_sys_stat_t));
                i = BQ76930_I2C_RETRIES;
            }
        }

        
        
        if (++i > BQ76930_I2C_RETRIES) return eBike_err;
    }
}
eBike_err_t bq76930_write_sys_stat(bq76930_sys_stat_t sys_stat) {
    eBike_err_t eBike_err;

    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;
    uint8_t message[] = {address_byte, BQ76930_SYS_STAT, bq76930_serialize_sys_stat(sys_stat), 0x00};

    message[3] = crc8(message, 3);

    int i = 0;
    while (true) {

        eBike_err = bq76930_write_register(BQ76930_SYS_STAT, (message + 2), 2);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C write SYS_STAT failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, i + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }else{
            printf("[BMS] - I2C wrote %02X to SYS_STAT.\n", message[2]);
            i = BQ76930_I2C_RETRIES;
        }

        if (++i > BQ76930_I2C_RETRIES) return eBike_err;
    }
}

eBike_err_t bq76930_read_sys_ctrl1(bq76930_sys_ctrl1_t* sys_ctrl1_ptr) {
    eBike_err_t eBike_err;
    
    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;
    
    int i = 0;
    while (true) {

        uint8_t* response = calloc(2, 1);
        eBike_err = bq76930_read_register(BQ76930_SYS_CTRL1, response, 2);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C read SYS_CTRL1 failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, i + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }else{

            uint8_t crc_bytes[] = {address_byte, *response};
            uint8_t crc = crc8(crc_bytes, 2);

            if (crc != *(response + 1)) {
                eBike_err.eBike_err_type = EBIKE_BMS_I2C_CRC_MISMATCH;

                char* log_message = calloc(1000, 1);
                sprintf(log_message, "[BMS] - I2C read SYS_CTRL1 failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                     "        Expected CRC %02X, got %02X\n"
                                     "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, crc, *(response + 1), i + 1);
                eBike_log_add(log_message, strlen(log_message));
                free(log_message);
            }else{
                bq76930_sys_ctrl1_t sys_crtl1 = bq76930_parse_sys_ctrl1(*response);
                memcpy(sys_ctrl1_ptr, &sys_crtl1, sizeof(bq76930_sys_ctrl1_t));
                i = BQ76930_I2C_RETRIES;
            }
        }

        free(response);
        if (++i > BQ76930_I2C_RETRIES) return eBike_err;
    }
}
eBike_err_t bq76930_write_sys_ctrl1(bq76930_sys_ctrl1_t sys_ctrl1) {
    eBike_err_t eBike_err;

    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;
    uint8_t message[] = {address_byte, BQ76930_SYS_CTRL1, bq76930_serialize_sys_ctrl1(sys_ctrl1), 0x00};

    message[3] = crc8(message, 3);

    int i = 0;
    while (true) {

        eBike_err = bq76930_write_register(BQ76930_SYS_CTRL1, (message + 2), 2);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C write SYS_CTRL1 failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, i + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }else{
            printf("[BMS] - I2C wrote %02X to SYS_CTRL1.\n", message[2]);
            i = BQ76930_I2C_RETRIES;
        }

        if (++i > BQ76930_I2C_RETRIES) return eBike_err;
    }
}

eBike_err_t bq76930_read_sys_ctrl2(bq76930_sys_ctrl2_t* sys_ctrl2_ptr) {
    eBike_err_t eBike_err;
    
    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;
    
    int i = 0;
    while (true) {

        uint8_t* response = calloc(2, 1);
        eBike_err = bq76930_read_register(BQ76930_SYS_CTRL2, response, 2);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C read SYS_CTRL2 failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, i + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }else{

            uint8_t crc_bytes[] = {address_byte, *response};
            uint8_t crc = crc8(crc_bytes, 2);

            if (crc != *(response + 1)) {
                eBike_err.eBike_err_type = EBIKE_BMS_I2C_CRC_MISMATCH;

                char* log_message = calloc(1000, 1);
                sprintf(log_message, "[BMS] - I2C read SYS_CTRL2 failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                     "        Expected CRC %02X, got %02X\n"
                                     "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, crc, *(response + 1), i + 1);
                eBike_log_add(log_message, strlen(log_message));
                free(log_message);
            }else{
                bq76930_sys_ctrl2_t sys_crtl2 = bq76930_parse_sys_ctrl2(*response);
                memcpy(sys_ctrl2_ptr, &sys_crtl2, sizeof(bq76930_sys_ctrl2_t));
                i = BQ76930_I2C_RETRIES;
            }
        }

        free(response);
        if (++i > BQ76930_I2C_RETRIES) return eBike_err;
    }
}
eBike_err_t bq76930_write_sys_ctrl2(bq76930_sys_ctrl2_t sys_ctrl2) {
    eBike_err_t eBike_err;

    uint8_t address_byte = (CONFIG_I2C_SLAVE_ADDRESS << 1) | I2C_MASTER_READ;
    uint8_t message[] = {address_byte, BQ76930_SYS_CTRL2, bq76930_serialize_sys_ctrl2(sys_ctrl2), 0x00};

    message[3] = crc8(message, 3);

    int i = 0;
    while (true) {

        eBike_err = bq76930_write_register(BQ76930_SYS_CTRL2, (message + 2), 2);

        if (eBike_err.eBike_err_type != EBIKE_OK) {
            char* log_message = calloc(1000, 1);
            sprintf(log_message, "[BMS] - I2C write SYS_CTRL2 failed: eBike_err: %s (%i) esp_err: %s (%i)\n"
                                 "        Attempt: %i\n", eBike_err_to_name(eBike_err.eBike_err_type), eBike_err.eBike_err_type, esp_err_to_name(eBike_err.esp_err), eBike_err.esp_err, i + 1);
            eBike_log_add(log_message, strlen(log_message));
            free(log_message);
        }else{
            printf("[BMS] - I2C wrote %02X to SYS_CTRL2.\n", message[2]);
            i = BQ76930_I2C_RETRIES;
        }

        if (++i > BQ76930_I2C_RETRIES) return eBike_err;
    }
}
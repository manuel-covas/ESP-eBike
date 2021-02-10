#ifndef EBIKE_BLE_IO_H
#define EBIKE_BLE_IO_H

#include <esp_gatts_api.h>
#include <bq76930.h>
#include <eBike_bms.h>

typedef enum {
    EBIKE_COMMAND_NONE,
    EBIKE_COMMAND_LOG_RETRIEVE,
    EBIKE_COMMAND_GET_SETTINGS,
    EBIKE_COMMAND_GET_ADC_CHARACTERISTICS,
    EBIKE_COMMAND_SYSTEM_STATS_STREAM,
    EBIKE_COMMAND_SYSTEM_STATS_UPDATE,
    EBIKE_COMMAND_AUTH_GET_CHALLENGE,
    EBIKE_COMMAND_AUTHED_COMMAND,
    EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS
}
eBike_command_t;


typedef struct __attribute__((__packed__)) eBike_system_stats_t {
    bq76930_sys_stat_t sys_stat;
    bq76930_cellbal_t cell_balancing;
    bq76930_sys_ctrl_1_t sys_ctrl1;
    bq76930_sys_ctrl_2_t sys_ctrl2;
    eBike_cell_voltages_t cell_voltages;
    double pack_voltage;
    double pack_current;
    double ts1_voltage;
    double ts2_voltage;
    double throttle_percentage;
}
eBike_system_stats_t;

typedef struct __attribute__((__packed__)) eBike_authed_command_t {
    uint8_t* command;
    uint16_t length;
    uint8_t* signature;
    uint16_t signature_length;
}
eBike_authed_command_t;


bool system_stats_stream_enabled;

void eBike_ble_io_recieve(struct gatts_write_evt_param* p);

#endif
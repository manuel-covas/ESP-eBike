#ifndef EBIKE_BLE_IO_H
#define EBIKE_BLE_IO_H

#include <esp_gatts_api.h>

typedef enum {
    EBIKE_COMMAND_NONE,
    EBIKE_COMMAND_LOG_RETRIEVE,
    EBIKE_COMMAND_GET_SETTINGS,
    EBIKE_COMMAND_GET_ADC_CHARACTERISTICS,
    EBIKE_COMMAND_AUTH_GET_CHALLENGE,
    EBIKE_COMMAND_AUTHED_COMMAND,
    EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS
}
eBike_command_t;


typedef struct __attribute__((__packed__)) eBike_authed_command_t {
    uint8_t* command;
    uint16_t length;
    uint8_t* signature;
    uint16_t signature_length;
}
eBike_authed_command_t;


void eBike_ble_io_recieve(struct gatts_write_evt_param* p);

#endif
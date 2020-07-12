#ifndef EBIKE_BLE_IO_H
#define EBIKE_BLE_IO_H

typedef enum {
    EBIKE_COMMAND_NONE,
    EBIKE_COMMAND_LOG_RETRIEVE
}
eBike_command_t;

typedef struct eBike_command_state_t {
    uint16_t current_index;
    eBike_command_t current_command;
}
eBike_command_state_t;


void eBike_ble_io_recieve(uint8_t* data, uint16_t length);

#endif
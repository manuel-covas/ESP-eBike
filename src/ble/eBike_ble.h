#ifndef EBIKE_BLE_H
#define EBIKE_BLE_H

#include <eBike_err.h>


eBike_err_t eBike_ble_init();

typedef struct __attribute__((__packed__)) eBike_response_t {
    uint8_t eBike_response;
    eBike_err_t eBike_err;
}
eBike_response_t;

typedef struct eBike_ble_message_t {
    uint8_t* data;
    size_t length;
    bool is_indicate;
} eBike_ble_message_t;

eBike_err_t eBike_queue_ble_message(eBike_response_t* command_response, void* response_data, size_t response_data_length, bool is_indicate);

#endif
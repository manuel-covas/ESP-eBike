#ifndef EBIKE_BLE_H
#define EBIKE_BLE_H

#include <eBike_err.h>

eBike_err_t eBike_ble_init();


typedef struct eBike_ble_message_t {
    uint8_t* data;
    size_t length;
    bool is_indicate;
} eBike_ble_message_t;

eBike_err_t eBike_queue_ble_message(uint8_t* data, size_t length, bool is_indicate);

#endif
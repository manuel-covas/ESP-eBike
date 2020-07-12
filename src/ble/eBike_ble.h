#ifndef EBIKE_BLE_H
#define EBIKE_BLE_H

#include <eBike_err.h>

eBike_err_t eBike_ble_init();
eBike_err_t eBike_ble_tx(uint8_t* data, uint16_t length);

#endif
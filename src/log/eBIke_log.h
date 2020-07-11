#ifndef EBIKE_LOG_H
#define EBIKE_LOG_H

#include <eBike_err.h>

eBike_err_t eBike_ble_log_init();
void eBike_ble_log(char* message, uint16_t length);

#endif
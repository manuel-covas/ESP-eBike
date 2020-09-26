#ifndef EBIKE_UTIL_H
#define EBIKE_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <eBike_nvs.h>

uint8_t crc8(uint8_t* ptr, uint8_t len);
void crc32(const void *data, size_t n_bytes, uint32_t* crc);

char* eBike_print_settings(eBike_settings_t eBike_settings);

#endif
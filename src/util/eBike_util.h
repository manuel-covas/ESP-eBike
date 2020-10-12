#ifndef EBIKE_UTIL_H
#define EBIKE_UTIL_H

#include <stdint.h>
#include <stdlib.h>
#include <eBike_nvs.h>

uint8_t crc8(uint8_t* ptr, uint8_t len);
unsigned int xcrc32(const unsigned char *buf, int len);

char* eBike_print_settings(eBike_settings_t eBike_settings, char* header_message);
void bytes_to_hex(uint8_t* pointer, size_t length);

#endif
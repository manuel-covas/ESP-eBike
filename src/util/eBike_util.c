#include <stdint.h>
#include <stdlib.h>
#include <sdkconfig.h>
#include <eBike_nvs.h>

uint8_t crc8(uint8_t* ptr, uint8_t len) {
    uint8_t key = 0x07;
	uint8_t i;
	uint8_t crc=0;

	while(len--!=0) {
		for(i=0x80; i!=0; i/=2) {
			if((crc & 0x80) != 0) {
				crc *= 2;
				crc ^= key;
			}else{
				crc *= 2;
            }
			if((*ptr & i)!=0) crc ^= key;
		} ptr++;
	} return(crc);
}

uint32_t crc32_for_byte(uint32_t r) {
    for(int j = 0; j < 8; ++j)
        r = (r & 1? 0: (uint32_t)0xEDB88320L) ^ r >> 1;
    return r ^ (uint32_t)0xFF000000L;
}

void crc32(const void *data, size_t n_bytes, uint32_t* crc) {
    static uint32_t table[0x100];

    if(!*table)
        for(size_t i = 0; i < 0x100; ++i)
            table[i] = crc32_for_byte(i);

    for(size_t i = 0; i < n_bytes; ++i)
        *crc = table[(uint8_t)*crc ^ ((uint8_t*)data)[i]] ^ *crc >> 8;
}


char* eBike_print_settings(eBike_settings_t eBike_settings) {

    double overcurrent_amps = (eBike_settings.bq76930_double_thresholds + 1) * (eBike_settings.bq76930_overcurrent_threshold);

    char* message;
    int result = asprintf(&message, "[System] - Loaded settings form NVS:\n"
                                    "    ");
}
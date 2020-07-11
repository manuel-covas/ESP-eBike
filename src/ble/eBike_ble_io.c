#include <stdio.h>
#include <string.h>
#include <eBike_err.h>
#include <eBike_ble_io.h>

void eBike_ble_find_command(uint8_t* data, uint16_t current_index, uint16_t length);
uint8_t* eBike_flip_array(uint8_t* flipped_data, uint16_t length);


uint8_t EBIKE_COMMAND_HEADER_HIGH = 0x29;
uint8_t EBIKE_COMMAND_HEADER_LOW  = 0x26;

eBike_command_state_t command_state = {
    .current_index = 0,
    .current_command = EBIKE_COMMAND_NONE
};


void eBike_ble_io_recieve(uint8_t* data, uint16_t length) {
    if (data == NULL || length < 1) return;

 /* uint8_t* data = eBike_flip_array(data, length);
    if (data == NULL) {
        printf("[BLE] - Couldn't malloc %i bytes to hold flipped message.\n", length);
        return;
    } */

    if (command_state.current_command == EBIKE_COMMAND_NONE)
        eBike_ble_find_command(data, 0, length);

    printf("[BLE] - Running command %i from index %i.\n", command_state.current_command, command_state.current_index);

    switch (command_state.current_command) {

        case EBIKE_COMMAND_RETRIEVE_LOG:
            
        break;
    
        default:
        break;
    }
}


void eBike_ble_find_command(uint8_t* data, uint16_t current_index, uint16_t length) {
    if (length < 2) return;
    
    for (int i = current_index; i < length - 1; i++) {
        if (memcmp(data + i,     &EBIKE_COMMAND_HEADER_HIGH, sizeof(uint8_t)) == 0 &&
            memcmp(data + i + 1, &EBIKE_COMMAND_HEADER_LOW,  sizeof(uint8_t)) == 0) {
            
            command_state.current_index = i + 2;
            command_state.current_command = *(data + i + 2);
            return;
        }
    }
}

/*
uint8_t* eBike_flip_array(uint8_t* flipped_data, uint16_t length) {
    uint8_t* result = malloc(sizeof(uint8_t) * length); if (result == NULL) return NULL;

    for (int i = 0; i < length; i++) {
        memcpy(result + length - 1 - i, flipped_data + i, sizeof(uint8_t));
    }
    return result;
} */
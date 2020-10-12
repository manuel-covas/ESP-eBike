#ifndef EBIKE_BLE_IO_H
#define EBIKE_BLE_IO_H

typedef enum {
    EBIKE_COMMAND_NONE,
    EBIKE_COMMAND_LOG_RETRIEVE,
    EBIKE_COMMAND_GET_SETTINGS,
    EBIKE_COMMAND_AUTH_GET_CHALLENGE,
    EBIKE_COMMAND_AUTHED_COMMAND,
    EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS
}
eBike_command_t;


typedef struct eBike_authed_command_t {
    uint8_t* authed_command;
    uint8_t authed_command_length;
    uint8_t* signature;
    uint16_t signature_length;
}
eBike_authed_command_t;

typedef struct eBike_command_response_t {
    uint8_t eBike_command;
    esp_err_t esp_err;
    eBike_err_type_t eBike_err_type;
}
eBike_command_response_t;


void eBike_ble_io_recieve(void* p);

#endif
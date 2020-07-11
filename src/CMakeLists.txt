# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES app_update bt)

set(COMPONENT_SRCS "ESP-eBike.c" "ota/eBike_ota.c" "ble/eBike_ble.c" "ble/eBike_ble_io.c" "err/eBike_err.c" "nvs/eBike_nvs.c" "gpio/eBike_gpio.c" "log/eBike_log.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "ota" "ble" "nvs" "log" "err" "gpio")

register_component()

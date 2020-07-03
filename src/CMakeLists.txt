# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES)
set(COMPONENT_PRIV_REQUIRES app_update bt)

set(COMPONENT_SRCS "ESP-eBike.c" "ota/eBike_ota.c" "ble/eBike_ble.c" "err/eBike_err.c" "nvs/eBike_nvs.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "ota" "ble" "nvs" "err")

register_component()

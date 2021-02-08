# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES esp_adc_cal)
set(COMPONENT_PRIV_REQUIRES bt)

set(COMPONENT_SRCS "ESP-eBike.c"
                   "util/eBike_util.c"
                   "ble/eBike_ble.c"
                   "ble/eBike_ble_io.c"
                   "err/eBike_err.c"
                   "nvs/eBike_nvs.c"
                   "gpio/eBike_gpio.c"
                   "log/eBike_log.c"
                   "auth/eBike_auth.c"
                   "bms/eBike_bms.c"
                   "bms/bq76930.c"
                   "adc/eBike_adc.c")
set(COMPONENT_ADD_INCLUDEDIRS "." "util" "ble" "nvs" "log" "err" "gpio" "auth" "bms" "adc")

register_component()

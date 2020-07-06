# ESP-eBike
ESP32 Firmware for a DIY eBike BMS and control board.

# Project Configuration
The default values are set according to this circuit board. Configuration: `idf.py menuconfig`

# Circuit Board
EasyEDA project: https://easyeda.com/manuel.p.covas/esp-ebike-v2

When designing this PCB I connected the buzzer's MOSFET to pin 5 (name SENSOR_VN, GPIO 39) of the ESP-32-WROOM module but missed that in it's datasheet (https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) it is stated that this pin can only be used as an input.
I had to pick up the iron again to add a wire and bridge it to another pin that could do the job and had been left unused. I chose pin 14 (GPIO 12) but others will do too.

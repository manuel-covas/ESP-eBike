# ESP-eBike
ESP32 Firmware for a DIY eBike BMS and control board.

# Project Configuration
The default values are set according to this circuit board. Configuration: `idf.py menuconfig`

# Circuit Board
EasyEDA project: https://easyeda.com/manuel.p.covas/esp-ebike-v2

Beware if you plan to manufacture this PCB, when designing it the following mistakes were made:
 - Buzzer's MOSFET is connected to pin 5 (SENSOR_VN, GPIO 39)
 - I2C lines (SDA and SCL) to the BQ76930 are connected to pin 6 and 7 (GIPO 34 and GPIO 35)
 
These pins are clearly stated as input only in the [datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) but I didn't look for that there since GPIO literally has ouput in it's name.

I added the following corrections with an iron (default values for project configuration):
 - Buzzer: pin 5 bridged to pin 14 (GPIO 12)
 - I2C: pin 6 (SDA) and 7 (SCL) bridged to pin 13 and 16 (GPIO14 and GPIO13)

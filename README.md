# ESP-eBike
ESP32 Firmware for a DIY eBike BMS and control board.
 
## Project Configuration
The default values are set according to this circuit board.\
Configuration: `idf.py menuconfig`

## Circuit Board
EasyEDA project: https://easyeda.com/manuel.p.covas/esp-ebike-v2

Beware if you plan to manufacture this PCB, when designing it the following mistakes were made:
- Buzzer's MOSFET is connected to pin 5 (SENSOR_VN, GPIO 39)
- I2C lines (SDA and SCL) to the BQ76930 are connected to pin 6 and 7 (GIPO 34 and GPIO 35)

These pins are clearly stated as input only in the [datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) but I didn't look for that there since GPIO literally has ouput in it's name.

I added the following corrections with an iron (default values for project configuration):
- Buzzer: pin 5 bridged to pin 14 (GPIO 12)
- I2C: pin 6 (SDA) and 7 (SCL) bridged to pin 13 and 16 (GPIO14 and GPIO13)

## Bluetooth Low Energy (BLE) Interface
The ESP32 can be intercated with through a BLE GATT server that allows for commands issuing as well data retreival.

### Advertisement Packets
BLE advertisment packets will be sent out periodically whenever there is no active connection.
The eBike BLE service's 16-bit UUID is included in these packets as defined in [src/ble/eBike_ble.c](https://github.com/manuel-covas/ESP-eBike/blob/master/src/ble/eBike_ble.c)

### Connecting
Data is exchanged by writing to the eBike-RX characteristic and receiving BLE indications from the eBike-TX characteristic. These are contained in the eBike BLE service.

**eBike BLE Service** `UUID: 2926 (16-bit) or 00002926-0000-1000-8000-00805f9b34fb (128-bit)`
- **eBike TX Characteristic** `UUID: aa01 or 0000aa01-0000-1000-8000-00805f9b34fb`\
Properties: *INDICATE*
- **eBike RX Characteristic** `UUID: ab01 or 0000ab01-0000-1000-8000-00805f9b34fb`\
Properties: *WRITE*

Uppon finishing service discovery the connecting device should:
- Set the connection's MTU to `517` (*needed*)
- Set the connection priority to `HIGH` (*better performance when streaming stats*)
- Subscribe to / enable indications from the eBike TX Characteristic (*often needed*)

### Format
Each write / indicate event contains only one, full, packet.\
Packets are formatted as follows when writing:

|                 | Command Byte      | Data (optional) |
|:---------------:|:------------------:|:---------------:|
| **Length**      | 1 byte             | Command specific length |
| **Description** | Identifies command | Optional data bytes |

Packets are formatted as follows when receiving:

|   | Response Byte | ESP Error | eBike Error | Data (optional) |
|:-:|:-------------:|:---------:|:-----------:|:---------------:|
| **Length** | 1 byte | 4 bytes (LSB) | 4 bytes (LSB) | Response dependent |
| **Description** | Identifies response type | ESP error integer | eBike error integer | Optional response data |

### Commands
The BLE commands are listed below:

- **Retreive Log** - `0x01 (EBIKE_COMMAND_LOG_RETRIEVE)`\
Used to retreive the BLE system log. It is less extensive than the log written to UART_0 (printf).\
One or more `0x01` responses containg the text will be sent back.\
No data for this command, extra bytes will be ignored.

- **Get Settings** - `0x02 (EBIKE_COMMAND_GET_SETTINGS)`\
Used to retrieve the eBike settings stored in NVS on the ESP32.\
One `0x02` response will be sent back containing the data.\
No data for this command, extra bytes will be ignored.

- **Get ADC Characteristics** - `0x03 (EBIKE_COMMAND_GET_ADC_CHARACTERISTICS)`\
Used to retrieve the on-board BQ76930's ADC characteristics.\
One `0x03` response will be sent back containing the data.\
No data for this command, extra bytes will be ignored.

- **Get Authentication Challenge** - `0x04 (EBIKE_COMMAND_AUTH_GET_CHALLENGE)`\
Used to get the currently active authentication challenge.\
One `0x04` response will be sent back containing the challenge.\
No data for this command, extra bytes will be ignored.

- **Run Authenticated Command** - `0x05 (EBIKE_COMMAND_AUTHED_COMMAND)`\
Used to execute an Authenticated Command (see below).\
One `0x05` response will be sent back indicating the outcome.\
Extra data must be sent for this command. See below.

### Authentication
Some BLE commands can't be run without authentication. These are *Authenticated Commands*.

This project is meant to be built with the public key of an RSA keypair in it's configuration.\
To run an Authenticated Command one must suffix the packet with the bytes of the currently active authentication challenge,
take an SHA-256 hash of this concatenation and sign it with the private key of the keypair in use.\
This signature can then be used as a parameter for the **Run Authenticated Command** command.

Thus the structure of **Run Authenticated Command** - `0x05 (EBIKE_COMMAND_AUTHED_COMMAND)` is as follows:

|                 | Command Byte | Authenticated Command length      | Authenticated Command          | RSA Signature length              | RSA Signature |
|:---------------:|:------------:|:---------------------------------:|:------------------------------:|:---------------------------------:|:-------------:|
| **Length**      | 1 byte       | 2 bytes (LSB)                     | CMD_LEN bytes                  | 2 bytes (LSB)                     | SIG_LEN bytes |
| **Description** | **0x05**     | Unsigned 16 bit integer (CMD_LEN) | Command to run (nested packet) | Unsigned 16 bit integer (SIG_LEN) | RSA Signature |

### Authenticated Commands
The Authenticated BLE commands are listed below:

- **Put Settings** - `0x06 (EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS)`\
Used to overwrite the current settings.\
One `0x06` response will be sent back indicating the outcome.\
The data bytes must consist of, at least, sizeof(eBike_settings_t) bytes which correspond, in the order defined in [eBike_nvs.h](https://github.com/manuel-covas/ESP-eBike/blob/master/src/nvs/eBike_nvs.h), to the bytes of an eBike_settings_t struct.\
Extra bytes will be ignored.


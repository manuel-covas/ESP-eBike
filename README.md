
    

# ESP-eBike
ESP32 Firmware for a DIY eBike BMS and control board.\
This software is licensed under BY-NC-SA 4.0, see [LICENSE.md](LICENSE.md), for any inquiry feel free to contact me at manuel.p.covas@gmail.com
 
## Build Configuration
With a correctly setup ESP-IDF environment, access the build configuration menu with the `idf.py menuconfig` command.

> Make sure that the **Bluetooth** component is enabled in the build system.\
> You can find the option in the `(Top) → Component config → Bluetooth` menu section.

The following items are located in the `(Top) → ESP-eBike Configuration` menu section:

| Fied Name                                        | Default Value | Description |
|:-------------------------------------------------|:--------------|:-------------------------------------------------------------------------------------------|
| BLE Device Name                                  | `"ESP-eBike"` | The Bluetooth device name to use.                                                          |
| Authentication RSA Public Key                    | `Empty`       | The RSA public key used for authentication                                                 |
| Crypto Challenge Length (bytes)                  | `64`          | The amount of random bytes to use for the crypto challenge.                                |
| Current Sense Resistor Value (mOhm)              | `1`           | The value, in milliohms, of the current sense resistor.                                    |
| Power Output 1 GPIO Number                       | `16`          | The GPIO number of the pin which activates Power Output 1.                                 |
| Power Output 2 GPIO Number                       | `17`          | The GPIO number of the pin which activates Power Output 2.                                 |
| Power Output 3 GPIO Number                       | `5`           | The GPIO number of the pin which activates Power Output 3.                                 |
| Power Output 4 GPIO Number                       | `18`          | The GPIO number of the pin which activates Power Output 4.                                 |
| Power Output 5 GPIO Number                       | `19`          | The GPIO number of the pin which activates Power Output 5.                                 |
| ADC1 Throttle Channel Number                     | `0`           | The ADC1 channel number to which the throttle analog signal is connected.                  |
| ADC1 Multisampling Count                         | `64`          | The amount of samples to take the average of for each ADC read.                            |
| Magnet GPIO Number                               | `33`          | The GPIO number of the pin from which to count pulses from and calculate speed.            |
| ESC PWM GPIO Number                              | `25`          | The GPIO number of the output pin for the PWM ESC control signal.                          |
| ESC PWM Frequency (Hz)                           | `50`          | Frequency in Hertz to use when generating the ESC PWM control signal.                      |
| Buzzer GPIO Number                               | `12`          | The GPIO number of the pin that activates the buzzer.                                      |
| Fault LED GPIO Number                            | `14`          | The GPIO number of the pin that activates the FAULT indicator led.                         |
| I2C Clock Frequency (Hz)                         | `50000`       | The frequency to clock the I2C line at.                                                    |
| I2C Slave Address                                | `0x08`        | The slave address of the I2C connected BQ76930 chip.                                       |
| I2C SCL GPIO Number                              | `27`          | The GPIO number of the pin to use for the I2C clock line.                                  |
| I2C SDA GPIO Number                              | `26`          | The GPIO number of the pin to use for the I2C data line.                                   |
| BLE Log Buffer Size (bytes)                      | `2048`        | The size, in bytes, to use for the buffer of text logs that can be read through bluetooth. |
| System Sats Minimum Update Period (Milliseconds) | `200`         | The minimum period, in milliseconds, between updates of the BLE System Stats stream.       |

The default values are set according to this circuit board.

## Circuit Board
EasyEDA project: https://easyeda.com/manuel.p.covas/esp-ebike

## Bluetooth Low Energy (BLE) Interface
The ESP32 can be intercated with through a BLE GATT server that allows for command issuing as well as data retreival.

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

|                 | Command Byte       | Data (optional)         |
|:---------------:|:------------------:|:-----------------------:|
| **Length**      | 1 byte             | Command specific length |
| **Description** | Identifies command | Optional data bytes     |

Packets are formatted as follows when receiving:

|                 | Response Byte            | eBike Error                 | Data (optional)        |
|:---------------:|:------------------------:|:---------------------------:|:----------------------:|
| **Length**      | 1 byte                   | `sizeof(eBike_err_t)` bytes | Response dependent     |
| **Description** | Identifies response type | eBike error struct          | Optional response data |


### Data Structures

- **eBike_err_t** - 8 bytes ([eBike_err.h](src/err/eBike_err.h))\
ESP-eBike error struct, contains one ESP-IDF error code and one ESP-eBike error code.\
For ESP-IDF errors refer to https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/esp_err.html\
For eBike errors see [ESP-eBike Errors](#esp-ebike-errors)\
Format:
    |                 | ESP-IDF Error              | ESP-eBike Error                   |
    |:---------------:|:--------------------------:|:---------------------------------:|
    | **Length**      | 4 bytes (LSB)              | 4 bytes (LSB)                     |
    | **Description** | Signed integer (esp_err_t) | Signed integer (eBike_err_type_t) |

- **eBike_settings_t** - 14 bytes ([eBike_nvs.h](src/nvs/eBike_nvs.h))\
ESP-eBike settings struct, contains general settings.\
This data is kept in NVS and can be modified through BLE.\
For interperting values regarding the BMS refer to [bq76930.c](src/bms/bq76930.c) and [BQ76930's datasheet](https://www.ti.com/lit/ds/symlink/bq76930.pdf)\
&#9888; **Beware that misconfiguring these values could lead to battery damage and / or a fire hazard!** &#9888;


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
Extra data must be sent for this command. See *Authentication*.\
Format:
    |                 | Command Byte | Authenticated Command length      | Authenticated Command          | RSA Signature length              | RSA Signature |
    |:---------------:|:------------:|:---------------------------------:|:------------------------------:|:---------------------------------:|:-------------:|
    | **Length**      | 1 byte       | 2 bytes (LSB)                     | CMD_LEN bytes                  | 2 bytes (LSB)                     | SIG_LEN bytes |
    | **Description** | `0x05`       | Unsigned 16 bit integer (CMD_LEN) | Command to run (nested packet) | Unsigned 16 bit integer (SIG_LEN) | RSA Signature |

### Authentication
Some BLE commands can't be run without authentication. These are *Authenticated Commands*.\
This project is meant to be built with the public key of an RSA keypair in it's configuration.\
The configured string should fit the PEM format with no newlines:
```
-----BEGIN PUBLIC KEY-----
<string in project configuration>
-----END PUBLIC KEY-----
```
An RSA keypair can be easily generated with OpenSSL, for example: 
```bash
openssl genrsa -out ESP-eBike.pem  # Generates an RSA private key in PEM format to ESP-eBike.pem
openssl rsa -in ESP-eBike.pem \
            -outform PEM      \
            -pubout           \
            -out ESP-eBike.pub     # Writes out the public key in PEM format to ESP-eBike.pub
cat ESP-eBike.pub                  # Print out the public key
```
To run an Authenticated Command one must suffix the nested packet (authenticated command to be run) with the bytes of the currently active authentication challenge,
take an SHA-256 hash of this concatenation and sign it with the private key of the keypair in use.\
This signature can then be used as a parameter for the **Run Authenticated Command** command.

So if the client desires to run the authenticated command: `01020304`\
And the active challenge is:
```
20F87B70EBF5C4C929CF6D8470D9A45283B5E6695C6B150F0328BEEBB37C1BD6434CFAEEED6E4BE704109090BC1DCB413EF63FDE831AD70A8D257F76572BAEBB
```
The content to hash comes out as:
```
0102030420F87B70EBF5C4C929CF6D8470D9A45283B5E6695C6B150F0328BEEBB37C1BD6434CFAEEED6E4BE704109090BC1DCB413EF63FDE831AD70A8D257F76572BAEBB
```
It's SHA-256 hash would be: `F785CB539B08542D88152F58F1139FBF32EA08B7CB250B97C300824BBA1005A3`\
And signing it with OpenSSL would look like:
```bash
echo "F785CB539B08542D88152F58F1139FBF32EA08B7CB250B97C300824BBA1005A3" | xxd -r -p - > message_hash
openssl rsautl -in message_hash -out message_signature -sign -inkey ESP-eBike.pem
```
Finally, the Authenticated Command would be:
```
050004010203040100<message_signature>
--++++--------++++-------------------
```
`05`: Command byte\
`0004`: Authenticated command length (4 bytes)\
`01020304`: Authenticated command to run (nested packet)\
`0100`: RSA Signature length (256 bytes)\
`<message_signature>`: RSA Signature (contents of message_signature file in this example)


### Authenticated Commands
The Authenticated BLE commands are listed below:

- **Put Settings** - `0x06 (EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS)`\
    Used to overwrite the eBike settings stored in NVS on the ESP32.\
    One `0x06` response will be sent back indicating the outcome.\
    Extra data must consist of the eBike_settings_t struct as defined in [eBike_nvs.h](src/nvs/eBike_nvs.h)\
    Extra bytes will be ignored.\
    Format:
    |                 | Command Byte | eBike Settings                   |
    |:---------------:|:------------:|:--------------------------------:|
    | **Length**      | 1 byte       | `sizeof(eBike_settings_t)` bytes |
    | **Description** | `0x06`       | *eBike settings*                 |
    

### Responses
The BLE responses are listed bellow:

- **Retreive Log Response** - `0x01 (EBIKE_COMMAND_LOG_RETRIEVE)`\
    Response to the Retrieve Log command.\
    Each time one of these is received it's log content should be appended to previously received text.\
    Format:
    |            | Response Byte | eBike Error                 | Response data      |
    |:----------:|:-------------:|:---------------------------:|:------------------:|
    | **Length** | 1             | `sizeof(eBike_err_t)` bytes | Response dependent |
    | **Value**  | `0x01`        | eBike error struct          | *Text data*        |

- **Get Settings Response** - `0x02 (EBIKE_COMMAND_GET_SETTINGS)`\
    Response to the Get Settings command.\
    Contains the `eBike_settings_t` struct, as defined in [eBike_nvs.h](src/nvs/eBike_nvs.h), if successful, no data otherwise.\
    Format:
    |            | Response Byte | eBike Error                 | Response data                              |
    |:----------:|:-------------:|:---------------------------:|:------------------------------------------:|
    | **Length** | 1             | `sizeof(eBike_err_t)` bytes | `sizeof(eBike_settings_t)` bytes or **0**  |
    | **Value**  | `0x02`        | eBike error struct          | *eBike settings if successful*             |

- **Get ADC Characteristics Response** - `0x03 (EBIKE_COMMAND_GET_ADC_CHARACTERISTICS)`\
    Response to the Get ADC Characteristics command.\
    Contains the `bq76930_adc_characteristics_t` struct.\
    Format:
    |            | Response Byte | eBike Error                 | Response data                                 |
    |:----------:|:-------------:|:---------------------------:|:---------------------------------------------:|
    | **Length** | 1             | `sizeof(eBike_err_t)` bytes | `sizeof(bq76930_adc_characteristics_t)` bytes |
    | **Value**  | `0x03`        | eBike error struct          | *Shunt value and ADC gain and offset in µV*   |

- **Get Authentication Challenge** - `0x04 (EBIKE_COMMAND_AUTH_GET_CHALLENGE)`\
    Response to the Get Authentication Challenge command.\
    Contains the currently active authentication challenge.\
    Format:
    |            | Response Byte | eBike Error                 | Response data                                           |
    |:----------:|:-------------:|:---------------------------:|:-------------------------------------------------------:|
    | **Length** | 1             | `sizeof(eBike_err_t)` bytes | `CONFIG_EBIKE_AUTH_CHALLENGE_LENGTH` bytes (default 64) |
    | **Value**  | `0x04`        | eBike error struct          | *The bytes of the currently active auth challenge*      |

- **Run Authenticated Command** - `0x05 (EBIKE_COMMAND_AUTHED_COMMAND)`\
    Response to the Run Authenticated Command command.\
    The `eBike_err_type` value in the `eBike_err` struct will indicate the outcome of the authentication.
    - `EBIKE_OK` means authentication succeeded and the nested command will be executed.
    - Any other value indicates failure.
    
    Format:
    |            | Response Byte | eBike Error                 | Response data      |
    |:----------:|:-------------:|:---------------------------:|:------------------:|
    | **Length** | 1             | `sizeof(eBike_err_t)` bytes | 0 bytes            |
    | **Value**  | `0x05`        | eBike error struct          | *No response data* |

- **Put Settings** - `0x06 (EBIKE_COMMAND_AUTHED_COMMAND_PUT_SETTINGS)`\
    Response to the Put Settings command.\
    The `eBike_err` struct will indicate the outcome.
    
    Format:
    |            | Response Byte | eBike Error                 | Response data      |
    |:----------:|:-------------:|:---------------------------:|:------------------:|
    | **Length** | 1             | `sizeof(eBike_err_t)` bytes | 0 bytes            |
    | **Value**  | `0x06`        | eBike error struct          | *No response data* |


### ESP-eBike Errors
These are the enums for ESP-eBike errors: ([eBike_err.h](src/err/eBike_err.h))
```c
typedef enum {
    EBIKE_OK = 0,                                     // Success.
    EBIKE_NVS_INIT_ERASE_FAIL,                        // NVS Init: Failed to erase flash chip.
    EBIKE_NVS_INIT_FAIL,                              // NVS Init: ESP-IDF nvs_flash_init() failed.
    EBIKE_NVS_INIT_OPEN_FAIL,                         // NVS Init: Failed to open NVS namespace EBIKE_NVS_NAMESPACE with mode NVS_READWRITE.
    EBIKE_GPIO_INIT_CONFIG_FAIL,                      // GPIO Init: ESP-IDF gpio_config() failed.
    EBIKE_BLE_INIT_OUTGOING_QUEUE_CREATE_FAIL,        // BLE Init: Outgoing message queue creation failed.
    EBIKE_BLE_INIT_OUTGOING_TASK_CREATE_FAIL,         // BLE Init: Outgoing message task creation failed.
    EBIKE_BLE_INIT_CONTROLLER_INIT_FAIL,              // BLE Init: Initializing bluetooth controller with BT_CONTROLLER_INIT_CONFIG_DEFAULT failed.
    EBIKE_BLE_INIT_ENABLE_CONTROLLER_FAIL,            // BLE Init: Bluetooth controller enabling failed.
    EBIKE_BLE_INIT_BLUEDROID_INIT_FAIL,               // BLE Init: Bluedroid stack initialization failed.
    EBIKE_BLE_INIT_BLUEDROID_ENABLE_FAIL,             // BLE Init: Bluedroid stack enabling failed.
    EBIKE_BLE_INIT_GAP_CALLBACK_REGISTER_FAIL,        // BLE Init: GAP callback function registration failed.
    EBIKE_BLE_INIT_GATTS_CALLBACK_REGISTER_FAIL,      // BLE Init: GATT server callback function registration failed.
    EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL,           // BLE Init: GATT server callback function registration failed.
    EBIKE_BLE_INIT_SET_BT_NAME_FAIL,                  // BLE Init: Failed to set device's bluetooth name.
    EBIKE_BLE_INIT_SET_ADV_DATA_FAIL,                 // BLE Init: Failed to set desired BLE advertising data.
    EBIKE_BLE_INIT_START_ADV_FAIL,                    // BLE Init: Failed to start BLE advertising.
    EBIKE_BLE_TX_NOT_CONNECTED,                       // BLE Transmit: An attempt to send data over BLE was made but no connection was active.
    EBIKE_BLE_TX_BAD_ARGUMENTS,                       // BLE Transmit: Incorrect parameters passed to eBike_ble_tx()
    EBIKE_BLE_TX_MALLOC_FAIL,                         // BLE Transmit: Malloc failed in eBike_queue_ble_message()
    EBIKE_BLE_TX_QUEUE_FAIL,                          // BLE Transmit: Failed to add BLE message to outgoing queue.
    EBIKE_BLE_COMMAND_TOO_SHORT,                      // BLE Command: Received data was too short.
    EBIKE_LOG_INIT_MALLOC_FAIL,                       // BLE Log Init: malloc for the log's buffer failed.
    EBIKE_LOG_TX_MALLOC_FAIL,                         // BLE Log Transmit: malloc for a log chunk failed in eBike_log_send()
    EBIKE_AUTH_INIT_MALLOC_FAIL,                      // Authentication Init: Malloc failed
    EBIKE_AUTH_INIT_PARSE_KEY_FAIL,                   // Authentication Init: Parsing of built in public key failed.
    EBIKE_AUTHED_COMMAND_FAIL,                        // Authentication: Signature verification failed.
    EBIKE_NVS_SETTINGS_GET_FAIL,                      // NVS Settings: Read from NVS failed.
    EBIKE_NVS_SETTINGS_PUT_FAIL,                      // NVS Settings: Write to NVS failed.
    EBIKE_NVS_SETTINGS_CRC_MISMATCH,                  // NVS Settings: CRC check failed. Could happen when reading or writing settings.
    EBIKE_BMS_INIT_I2C_CONFIG_FAIL,                   // BMS Init: I2C driver configuring failed.
    EBIKE_BMS_INIT_I2C_INSTALL_FAIL,                  // BMS Init: I2C driver activation failed.
    EBIKE_BMS_I2C_BUILD_COMMAND_FAIL,                 // BMS I2C Communication: Failure while preparing I2C command to communicate with BQ769x0.
    EBIKE_BMS_I2C_COMMAND_FAIL,                       // BMS I2C Communication: I2C data exchange failed. (not acknowledged or other)
    EBIKE_BMS_I2C_CRC_MISMATCH                        // BMS I2C Communication: BQ769x0 communication CRC mismatched. (explained in the chip's datasheet)
} eBike_err_type_t;
```

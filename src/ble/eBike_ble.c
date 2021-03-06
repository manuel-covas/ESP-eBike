#include <sdkconfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <eBike_err.h>
#include <eBike_log.h>
#include <eBike_ble_io.h>
#include <esp_bt.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <esp_gatt_common_api.h>
#include <esp_bt_defs.h>
#include <esp_bt_main.h>


#define  ESP_BT_CONTROLLER_MODE ESP_BT_MODE_BLE
uint32_t EBIKE_CONNECT_BEEP_DURATION_MS = 80;
#define  EBIKE_BLE_MAX_CONNECTION_INTERVAL 200                       // Calculation: EBIKE_BLE_MAX_CONNECTION_INTERVAL * 1.25ms
#define  EBIKE_BLE_APPEARANCE ESP_BLE_APPEARANCE_CYCLING_COMPUTER    // Device's advertised BLE appearance
#define  EBIKE_BLE_GATTS_APP_ID 0                                    // Multiple GATTS apps can be registered
#define  EBIKE_BLE_SERVICE_16BIT_UUID 0x2926                         // BLE Service UUID
#define  EBIKE_BLE_TX_CHAR_16BIT_UUID 0xAA01                         // BLE Tx Characteristic UUID
#define  EBIKE_BLE_TX_CHAR_DESCRIPTOR_16BIT_UUID 0x2902              // BLE Tx Characteristic Descriptor UUID
#define  EBIKE_BLE_RX_CHAR_16BIT_UUID 0xAB01                         // BLE Rx Characteristic UUID


esp_ble_adv_data_t advertising_data = {
    .set_scan_rsp = false,                          /*!< Set this advertising data as scan response or not*/
    .include_name = true,                           /*!< Advertising data include device name or not */
    .include_txpower = true,                        /*!< Advertising data include TX power */
    .min_interval = ESP_BLE_CONN_INT_MIN,
    .max_interval = EBIKE_BLE_MAX_CONNECTION_INTERVAL,
    .appearance = EBIKE_BLE_APPEARANCE,
    .manufacturer_len = 0,                          /*!< Manufacturer data length */
    .p_manufacturer_data = NULL,                    /*!< Manufacturer data point */
    .service_data_len = 0,                          /*!< Service data length */
    .p_service_data = NULL,                         /*!< Service data point */
    .service_uuid_len = 0,                          /*!< Service uuid length */
    .p_service_uuid = NULL,                         /*!< Service uuid array point */
    .flag = ESP_BLE_ADV_FLAG_GEN_DISC               /*!< Advertising flag of discovery mode, see BLE_ADV_DATA_FLAG detail */
};

esp_ble_adv_params_t advertising_parameters = {
    .adv_int_min = 0x0800,                  /*!< Minimum advertising interval for
                                                 undirected and low duty cycle directed advertising.
                                                 Range: 0x0020 to 0x4000 Default: N = 0x0800 (1.28 second)
                                                 Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec */
    .adv_int_max = 0x0800,                  /*!< Maximum advertising interval for
                                                 undirected and low duty cycle directed advertising.
                                                 Range: 0x0020 to 0x4000 Default: N = 0x0800 (1.28 second)
                                                 Time = N * 0.625 msec Time Range: 20 ms to 10.24 sec Advertising max interval */
    .adv_type = ADV_TYPE_IND,               /*!< Advertising type */
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,  /*!< Owner bluetooth device address type */
    .channel_map = ADV_CHNL_ALL
};

esp_gatt_if_t gatts_interface;


bool eBike_ble_connected = false;
uint16_t eBike_ble_service_handle;
uint16_t eBike_ble_connection_id;
uint16_t eBike_ble_rx_char_handle, eBike_ble_tx_char_handle;

esp_gatt_srvc_id_t eBike_ble_service_id = {
    .is_primary = true,
    .id = {
        .inst_id = 0x00,
        .uuid = {
            .len = ESP_UUID_LEN_16,
            .uuid = {
                .uuid16 = EBIKE_BLE_SERVICE_16BIT_UUID
            }
        }
    }
};

esp_bt_uuid_t eBike_ble_rx_char_id = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = EBIKE_BLE_RX_CHAR_16BIT_UUID
    }
};
esp_attr_value_t eBike_ble_rx_char_value = {
    .attr_max_len = ESP_GATT_MAX_ATTR_LEN,
    .attr_len = 0,
    .attr_value = NULL
};

esp_bt_uuid_t eBike_ble_tx_char_id = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = EBIKE_BLE_TX_CHAR_16BIT_UUID
    }
};
esp_attr_value_t eBike_ble_tx_char_value = {
    .attr_max_len = ESP_GATT_MAX_ATTR_LEN,
    .attr_len = 0,
    .attr_value = NULL
};

esp_bt_uuid_t eBike_ble_tx_char_descriptor_id = {
    .len = ESP_UUID_LEN_16,
    .uuid = {
        .uuid16 = EBIKE_BLE_TX_CHAR_DESCRIPTOR_16BIT_UUID
    }
};
esp_attr_value_t eBike_ble_tx_char_descriptor_value = {
    .attr_max_len = 2,
    .attr_len = 2,
    .attr_value = NULL
};

esp_attr_control_t esp_ble_char_auto_response = {
    .auto_rsp = ESP_GATT_AUTO_RSP
};


bool ble_advertising_ready = false;

void eBike_check_advertising_ready() {
    if (ble_advertising_ready) {
        esp_ble_gap_start_advertising(&advertising_parameters);            
    }else{
        ble_advertising_ready = true;
    }
}


void eBike_gap_callback(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    switch (event) {
    
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        eBike_check_advertising_ready();
    break;

    case ESP_GAP_BLE_SEC_REQ_EVT:
        printf("[BLE] - Denied bonding: %s\n", esp_err_to_name(esp_ble_gap_security_rsp(param->ble_security.auth_cmpl.bd_addr, false)));
    break;
    
    default:
        printf("[BLE] - GAP Event: %i\n", event);
    break;
    }
}


void eBike_gatts_callback(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {

    case ESP_GATTS_REG_EVT:
        if (param->reg.app_id != EBIKE_BLE_GATTS_APP_ID) { return; }
        if (param->reg.status != ESP_GATT_OK) {
            printf("[BLE] - Failed to register GATTS app. (GATT Status: %i)\n", param->reg.status);
            eBike_err_report((eBike_err_t) {
                .esp_err = (esp_err_t) param->reg.status,
                .eBike_err_type = EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL
            });
        }else{
            gatts_interface = gatts_if;
            printf("[BLE] - GATTS app registered. (gatts_interface: %i)\n", gatts_interface);
            printf("[BLE] - Creating BLE service: %s\n", esp_err_to_name(esp_ble_gatts_create_service(gatts_interface, &eBike_ble_service_id, 15)));
        }
    break;

    case ESP_GATTS_CREATE_EVT:
        eBike_ble_service_handle = param->create.service_handle;
        printf("[BLE] - Adding BLE TX characteristic: %s\n", esp_err_to_name(esp_ble_gatts_add_char(eBike_ble_service_handle,
                                                                                                    &eBike_ble_tx_char_id,
                                                                                                    ESP_GATT_PERM_READ,
                                                                                                    ESP_GATT_CHAR_PROP_BIT_INDICATE,
                                                                                                    &eBike_ble_tx_char_value,
                                                                                                    &esp_ble_char_auto_response)));
    break;

    case ESP_GATTS_ADD_CHAR_EVT:
        if (param->add_char.char_uuid.uuid.uuid16 == EBIKE_BLE_RX_CHAR_16BIT_UUID) {
            eBike_ble_rx_char_handle = param->add_char.attr_handle;
            printf("[BLE] - Starting BLE service: %s\n", esp_err_to_name(esp_ble_gatts_start_service(eBike_ble_service_handle)));
        }
        if (param->add_char.char_uuid.uuid.uuid16 == EBIKE_BLE_TX_CHAR_16BIT_UUID) {
            eBike_ble_tx_char_handle = param->add_char.attr_handle;
            printf("[BLE] - Adding BLE TX characteristic descriptor: %s\n", esp_err_to_name(esp_ble_gatts_add_char_descr(eBike_ble_service_handle,
                                                                                                                        &eBike_ble_tx_char_descriptor_id,
                                                                                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                                                                                                        &eBike_ble_tx_char_descriptor_value,
                                                                                                                        &esp_ble_char_auto_response)));
        }
    break;

    case ESP_GATTS_ADD_CHAR_DESCR_EVT:
        if (param->add_char_descr.descr_uuid.uuid.uuid16 == EBIKE_BLE_TX_CHAR_DESCRIPTOR_16BIT_UUID) {
            printf("[BLE] - Adding BLE RX characteristic: %s\n", esp_err_to_name(esp_ble_gatts_add_char(eBike_ble_service_handle,
                                                                                            &eBike_ble_rx_char_id,
                                                                                            ESP_GATT_PERM_WRITE,
                                                                                            ESP_GATT_CHAR_PROP_BIT_WRITE,
                                                                                            &eBike_ble_rx_char_value,
                                                                                            &esp_ble_char_auto_response)));
        }
    break;

    case ESP_GATTS_START_EVT:
        eBike_check_advertising_ready();
    break;


    case ESP_GATTS_CONNECT_EVT:
        eBike_ble_connection_id = param->connect.conn_id;
        eBike_ble_connected = true;
        eBike_ble_release_command_lock();
        eBike_beep(&EBIKE_CONNECT_BEEP_DURATION_MS);
    break;

    case ESP_GATTS_DISCONNECT_EVT:
        eBike_ble_connected = false;
        eBike_beep(&EBIKE_CONNECT_BEEP_DURATION_MS);
        vTaskDelay(EBIKE_CONNECT_BEEP_DURATION_MS*2 / portTICK_PERIOD_MS);
        eBike_beep(&EBIKE_CONNECT_BEEP_DURATION_MS);
        esp_ble_gap_start_advertising(&advertising_parameters);
    break;


    case ESP_GATTS_WRITE_EVT: ;
        struct gatts_write_evt_param* parameters = &(param->write);
        if (!parameters->is_prep && parameters->handle == eBike_ble_rx_char_handle) {
            eBike_ble_io_recieve(parameters);
        }
    break;


    case ESP_GATTS_MTU_EVT:
        printf("[BLE] - Setting MTU to %i: %s\n", param->mtu.mtu, esp_err_to_name(esp_ble_gatt_set_local_mtu(param->mtu.mtu)));
    break;

    default:
        printf("[BLE] - GATTS Event: %i\n", event);
    break;
    }
}


eBike_err_t eBike_ble_tx(uint8_t* data, uint16_t length) {
    eBike_err_t eBike_err;

    if (!eBike_ble_connected) {
        eBike_err.eBike_err_type = EBIKE_BLE_TX_NOT_CONNECTED;
        eBike_err.esp_err = ESP_OK;
        return eBike_err;
    }else if (data == NULL || length < 1) {
        eBike_err.eBike_err_type = EBIKE_BLE_TX_BAD_ARGUMENTS;
        eBike_err.esp_err = ESP_OK;
    }else{
        eBike_err.eBike_err_type = EBIKE_OK;
        eBike_err.esp_err = esp_ble_gatts_send_indicate(gatts_interface, eBike_ble_connection_id, eBike_ble_tx_char_handle, length, data, true);
    }

    return eBike_err;
}


eBike_err_t eBike_ble_init() {
    eBike_err_t eBike_err;
    eBike_err_type_t eBike_err_type;

    esp_bt_controller_config_t bt_controller_config = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    bt_controller_config.mode = ESP_BT_CONTROLLER_MODE;

    eBike_ble_tx_char_descriptor_value.attr_value = calloc(2, 1);

    EBIKE_HANDLE_ERROR(esp_bt_controller_init(&bt_controller_config), EBIKE_BLE_INIT_CONTROLLER_INIT_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(esp_bt_controller_enable(ESP_BT_CONTROLLER_MODE), EBIKE_BLE_INIT_ENABLE_CONTROLLER_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(esp_bluedroid_init(), EBIKE_BLE_INIT_BLUEDROID_INIT_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(esp_bluedroid_enable(), EBIKE_BLE_INIT_BLUEDROID_ENABLE_FAIL, eBike_err);
    
    EBIKE_HANDLE_ERROR(esp_ble_gap_register_callback(eBike_gap_callback), EBIKE_BLE_INIT_GAP_CALLBACK_REGISTER_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(esp_ble_gatts_register_callback(eBike_gatts_callback), EBIKE_BLE_INIT_GATTS_CALLBACK_REGISTER_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(esp_ble_gatts_app_register(EBIKE_BLE_GATTS_APP_ID), EBIKE_BLE_INIT_GATTS_APP_REGISTER_FAIL, eBike_err);

    EBIKE_HANDLE_ERROR(esp_ble_gap_set_device_name(CONFIG_BLE_NAME), EBIKE_BLE_INIT_SET_BT_NAME_FAIL, eBike_err);
    EBIKE_HANDLE_ERROR(esp_ble_gap_config_adv_data(&advertising_data), EBIKE_BLE_INIT_SET_ADV_DATA_FAIL, eBike_err);
    
    eBike_err_type = eBike_log_init().eBike_err_type;
    if (eBike_err_type != EBIKE_OK)
        printf("[BLE] - Failed to allocate %i bytes for BLE log buffer. (%s)\n", CONFIG_EBIKE_LOG_BUFFER_SIZE, eBike_err_to_name(eBike_err_type));

eBike_clean:
    return eBike_err;
}
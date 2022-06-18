#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_bt.h"
#include "esp_log.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"
#include "sdkconfig.h"

#include "send_payload.h"
#include "recv_payload.h"
#include "system_utils.h"
#include "ble_server.h"

#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

/* VARIABLES */
uint8_t *rv_data;
bool write_EVT = false;

uint8_t *tx_data, tx_len;
bool read_EVT = false;

bool is_Aconnected = false;
bool notificationA_enable = false;
bool keep_bt = true;

static uint8_t char1_str[] = {0x11,0x22,0x33};
static esp_gatt_char_prop_t a_property = 0;
static uint8_t adv_config_done = 0;
static prepare_type_env_t a_prepare_write_env;

static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};

//adv data
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = false,
    .min_interval = 0x0006, //slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, //slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// scan response data
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .appearance = 0x00,
    .manufacturer_len = 0, //TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data =  NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(adv_service_uuid128),
    .p_service_uuid = adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = 0x20,/*!< Minimum advertising interval for undirected and low duty cycle directed advertising.
 					            Range: 0x0020 to 0x4000
 					            Default: N = 0x0800 (1.28 second)
					            Time = N * 0.625 msec
					            Time Range: 20 ms to 10.24 sec */
    .adv_int_max        = 0x40,/*!< Maximum advertising interval for undirected and low duty cycle directed advertising.
					            Range: 0x0020 to 0x4000
					            Default: N = 0x0800 (1.28 second)
					            Time = N * 0.625 msec
					            Time Range: 20 ms to 10.24 sec */
    .adv_type           = ADV_TYPE_IND, /*ADV_IND, which is a generic, not directed to a particular central device and connectable type*/
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
    .channel_map        = ADV_CHNL_ALL,/*!< Advertising channel map */
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, /*allows both scan and connection requests from any central.*/
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_ID] = {
        .gatts_cb = gatts_profile_a_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

static esp_attr_value_t gatts_demo_char1_val =
{
    .attr_max_len = GATTS_CHAR_VAL_LEN_MAX, //max char length 64 (bytes asumo)
    .attr_len     = sizeof(char1_str),
    .attr_value   = char1_str,
};



/* FUNCTIONS */

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param){
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT: /*Once the advertising data have been set, the GAP event ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT is triggered*/
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
            /*The advertising data is the information that is shown to the client, while the advertising parameters are the configuration required by the GAP to execute.*/
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done == 0){
            esp_ble_gap_start_advertising(&adv_params);
            /*The advertising data is the information that is shown to the client, while the advertising parameters are the configuration required by the GAP to execute.*/
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT: /*If the advertising started successfully, an ESP_GAP_BLE_ADV_START_COMPLETE_EVT event is generated*/
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising start failed\n");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GATTS_TAG, "Advertising stop failed\n");
        } else {
            ESP_LOGI(GATTS_TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:/*The esp_ble_gap_update_conn_params() function triggers a GAP event ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT*/
        break;
    default:
        break;
    }
}

void write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    esp_gatt_status_t status = ESP_GATT_OK;
    ESP_LOGI(GATTS_TAG, "Need rsp %d", param->write.need_rsp);
    if (param->write.need_rsp) {
        ESP_LOGI(GATTS_TAG, "Is prep %d", param->write.is_prep);
        ESP_LOGI(GATTS_TAG, "Write len %d", param->write.len);
        if (param->write.is_prep){
            if (prepare_write_env->prepare_buf == NULL) {
                prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE*sizeof(uint8_t));
                prepare_write_env->prepare_len = 0;
                if (prepare_write_env->prepare_buf == NULL) {
                    ESP_LOGE(GATTS_TAG, "Gatt_server prep no mem\n");
                    status = ESP_GATT_NO_RESOURCES;
                }
            } else {
                if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
                    status = ESP_GATT_INVALID_OFFSET;
                } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
                    status = ESP_GATT_INVALID_ATTR_LEN;
                }
            }

            esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE; /*ESP_GATT_AUTH_REQ_NONE means that the client can write to this characteristic without needing to authenticate first*/
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(GATTS_TAG, "Send response error\n");
            }
            free(gatt_rsp);
            if (status != ESP_GATT_OK){
                ESP_LOGE(GATTS_TAG, "Status not ok");
                return;
            }
            memcpy(prepare_write_env->prepare_buf + param->write.offset,
                   param->write.value,
                   param->write.len);
            prepare_write_env->prepare_len += param->write.len;
        } else {
            // There is only one message with less size than MTU, no EXEC event
            esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, NULL);
            ESP_LOGI(GATTS_TAG, "Write len %d", param->write.len);
            if (param->write.len == 1) {
                ESP_LOGI(GATTS_TAG, "Writing Rv Data");
                // Store message in global variable
                rv_data = (uint8_t *) malloc(sizeof(uint8_t));
                *rv_data = param->write.value[0];
                keep_bt = false;
                write_EVT = true;
            }
        }
    }
}

void exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    ESP_LOGI(GATTS_TAG, "Entering exec with %d", param->exec_write.exec_write_flag);
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC) {
        // Store config
        keep_bt = false;
        parse_and_save_config(prepare_write_env->prepare_buf);
        ESP_LOGI(GATTS_TAG, "Stored config through BLE");
        // esp_log_buffer_hex(GATTS_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    } else {
        ESP_LOGI(GATTS_TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }

    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}


void gatts_profile_a_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT: /*The register application event is the first one that is triggered during the lifetime of the program*/
        gl_profile_tab[PROFILE_APP_ID].service_id.is_primary = true;
        gl_profile_tab[PROFILE_APP_ID].service_id.id.inst_id = 0x00;
        gl_profile_tab[PROFILE_APP_ID].service_id.id.uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_APP_ID].service_id.id.uuid.uuid.uuid16 = GATTS_SERVICE_UUID;

        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(DEVICE_NAME);
        if (set_dev_name_ret){
            ESP_LOGE(GATTS_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }
        //config adv data
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        /*The advertising data is the information that is shown to the client, while the advertising parameters are the configuration required by the GAP to execute.*/
        if (ret){
            ESP_LOGE(GATTS_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= adv_config_flag;
        //config scan response data
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret){
            ESP_LOGE(GATTS_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= scan_rsp_config_flag;

        esp_ble_gatts_create_service(gatts_if, &gl_profile_tab[PROFILE_APP_ID].service_id, GATTS_NUM_HANDLE);
        break;
    case ESP_GATTS_READ_EVT: { /* To manage the read events*/
        esp_gatt_rsp_t rsp;
        memset(&rsp, 0, sizeof(esp_gatt_rsp_t));

        read_EVT = true;
        rsp.attr_value.handle = param->read.handle;
        rsp.attr_value.len = tx_len;
        for(int i=0; i<tx_len; i++){
            rsp.attr_value.value[i] = tx_data[i];
        }

        esp_ble_gatts_send_response(gatts_if, param->read.conn_id, param->read.trans_id,
                                    ESP_GATT_OK, &rsp); /*This function is necessary if the auto response byte is set to NULL when creating the characteristic or descriptor.*/
        break;
    }
    case ESP_GATTS_WRITE_EVT: { /* To manage the write events*/
        ESP_LOGI(GATTS_TAG, "Gatt event handler write event");
        if (!param->write.is_prep){
            ESP_LOGI(GATTS_TAG, "GATT_WRITE_EVT a, app_id %d, value len %d :", param->reg.app_id, param->write.len);
            esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);

            if (gl_profile_tab[PROFILE_APP_ID].descr_handle == param->write.handle && param->write.len == 2){
                uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                ESP_LOGI(GATTS_TAG, "descr value %hu", descr_value);
                if (descr_value == 0x0001){
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_NOTIFY){
                        uint8_t notify_data[15];
                        for (int i = 0; i < sizeof(notify_data); ++i)
                        {
                            notify_data[i] = i%0xff;
                        }
                        //the size of notify_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_APP_ID].char_handle,
                                                sizeof(notify_data), notify_data, false);
                        notificationA_enable = true;
                    }
                }else if (descr_value == 0x0002){
                    if (a_property & ESP_GATT_CHAR_PROP_BIT_INDICATE){
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i%0xff;
                        }
                        //the size of indicate_data[] need less than MTU size
                        esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, gl_profile_tab[PROFILE_APP_ID].char_handle,
                                                sizeof(indicate_data), indicate_data, true);
                    }
                }
                else if (descr_value == 0x0000){
                    ESP_LOGI(GATTS_TAG, "notify/indicate disable ");
                    notificationA_enable = false;
                }else{
                    ESP_LOGE(GATTS_TAG, "unknown descr value");
                    esp_log_buffer_hex(GATTS_TAG, param->write.value, param->write.len);
                }

            }
        }
        write_event_env(gatts_if, &a_prepare_write_env, param);
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT: /*The client finishes the long write sequence by sending an Executive Write Request. This command triggers an ESP_GATTS_EXEC_WRITE_EVT event.*/
        ESP_LOGI(GATTS_TAG,"ESP_GATTS_EXEC_WRITE_EVT");
        esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
        exec_write_event_env(&a_prepare_write_env, param);
        break;
    case ESP_GATTS_MTU_EVT:
        ESP_LOGI(GATTS_TAG, "ESP_GATTS_MTU_EVT a, MTU %d", param->mtu.mtu);
        break;
    case ESP_GATTS_UNREG_EVT:
        break;
    case ESP_GATTS_CREATE_EVT: /*When a service is created successfully, an ESP_GATTS_CREATE_EVT event is triggered, and can be used to start the service and add characte ristics*/
        gl_profile_tab[PROFILE_APP_ID].service_handle = param->create.service_handle;
        gl_profile_tab[PROFILE_APP_ID].char_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_APP_ID].char_uuid.uuid.uuid16 = GATTS_CHAR_UUID;

        esp_ble_gatts_start_service(gl_profile_tab[PROFILE_APP_ID].service_handle);
        a_property = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY; /*Characteristic can be read, can be written and can notify value changes*/
        esp_err_t add_char_ret = esp_ble_gatts_add_char(gl_profile_tab[PROFILE_APP_ID].service_handle, &gl_profile_tab[PROFILE_APP_ID].char_uuid,
                                                        ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, /*To read and to write characteristic value is permitted*/
                                                        a_property,
                                                        &gatts_demo_char1_val, NULL);
        /*Finally, the characteristic is configured in a way that it is required to send a response manually every time the characteristic is read or written, instead of letting the stack auto respond. This is configured by setting the last parameter of the esp_ble_gatts_add_char() function, representing the attribute response control parameter, to ESP_GATT_RSP_BY_APP or NULL.*/
        if (add_char_ret){
            ESP_LOGE(GATTS_TAG, "add char failed, error code =%x",add_char_ret);
        }
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:
        break;
    case ESP_GATTS_ADD_CHAR_EVT: { /*Adding a characteristic to a service triggers an ESP_GATTS_ADD_CHAR_EVT event*/
        uint16_t length = 0;
        const uint8_t *prf_char;

        gl_profile_tab[PROFILE_APP_ID].char_handle = param->add_char.attr_handle;
        gl_profile_tab[PROFILE_APP_ID].descr_uuid.len = ESP_UUID_LEN_16;
        gl_profile_tab[PROFILE_APP_ID].descr_uuid.uuid.uuid16 = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
        esp_err_t get_attr_ret = esp_ble_gatts_get_attr_value(param->add_char.attr_handle,  &length, &prf_char);
        if (get_attr_ret == ESP_FAIL){
            ESP_LOGE(GATTS_TAG, "ILLEGAL HANDLE");
        }

        esp_err_t add_descr_ret = esp_ble_gatts_add_char_descr(gl_profile_tab[PROFILE_APP_ID].service_handle, &gl_profile_tab[PROFILE_APP_ID].descr_uuid,
                                                                ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, NULL, NULL);
        /*The parameters(esp_ble_gatts_add_char_descr)used are the service handle, the descriptor UUID, write and read permissions, an initial value and the auto response setting.*/
        /*The initial value for the characteristic descriptor can be a NULL pointer and the auto response parameter is set to NULL as well, which means that requests that require responses have to be replied manually.*/
        if (add_descr_ret){
            ESP_LOGE(GATTS_TAG, "add char descr failed, error code =%x", add_descr_ret);
        }
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT: /*Once the descriptor is added, the ESP_GATTS_ADD_CHAR_DESCR_EVT event is triggered,*/
        gl_profile_tab[PROFILE_APP_ID].descr_handle = param->add_char_descr.attr_handle;
        break;
    case ESP_GATTS_DELETE_EVT:
        break;
    case ESP_GATTS_START_EVT: /*The service is started using the esp_ble_gatts_start_service() function with the service handle previously generated. An ESP_GATTS_START_EVT event, which is used to print information, is triggered.*/
        break;
    case ESP_GATTS_STOP_EVT:
        break;
    case ESP_GATTS_CONNECT_EVT: { /*An ESP_GATTS_CONNECT_EVT is triggered when a client has connected to the GATT server*/
    /*The update connection parameters procedure needs to be done only once, therefore the Profile B connection event handler does not include it*/
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        
        //start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        is_Aconnected = true;
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        is_Aconnected = false;
        break;
    case ESP_GATTS_CONF_EVT: //When a confirmation is received ESP_GATTS_CONF_EVT is triggered
        if (param->conf.status != ESP_GATT_OK){
            esp_log_buffer_hex(GATTS_TAG, param->conf.value, param->conf.len);
        }
        break;
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param){
    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) { /*When an Application Profile is registered, an ESP_GATTS_REG_EVT event is triggered.*/
    /*The event is captured by the gatts_event_handler(), which used to store the generated interface in the profile table, and then the event is forwarded to the corresponding profile event handler.*/
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            ESP_LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d\n",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* If the gatts_if equal to profile A, call profile A cb handler,
     * so here call each profile's callback */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gatts_if == gl_profile_tab[idx].gatts_if) {
                if (gl_profile_tab[idx].gatts_cb) {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

/**
 * @brief Function to initiate controllers (needs to be called only once)
 */
void ble_controllers_init() {
  ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
  ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
}

/** @brief Function that initiates the BLE server
    *  @param  MTU define MTU (Maximum Transmission Unit)
    *  @return Does not return data but prints in case of error. It can be modified to return any error
    * */
void ble_server_init(uint16_t MTU){
    esp_err_t ret;

    // ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    // ret = esp_bt_controller_init(&bt_cfg);
    // if (ret) {
    //     ESP_LOGE(GATTS_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
    //     return;
    // }

    // ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    // if (ret) {
    //     ESP_LOGE(GATTS_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
    //     return;
    // }
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(GATTS_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_APP_ID);
    if (ret){
        ESP_LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(MTU);
    if (local_mtu_ret){
        ESP_LOGE(GATTS_TAG, "set local MTU failed, error code = %x", local_mtu_ret);
    }
}

void ble_server_deinit() {
    ESP_ERROR_CHECK(esp_bluedroid_disable());
    ESP_ERROR_CHECK(esp_bluedroid_deinit());
    is_Aconnected = false;
    notificationA_enable = false;
    keep_bt = true;
    write_EVT = false;
}


/** @brief Send data through the BLE protcol as notification or indication 
    *  @param  payload_len Length of the data to send
    *  @param  payload An array woth the data to send
    *  @param status_id The id of the system status being used
    *  @param protocol_id The id of the protocol of the message being sent 
    *  @param  type Indicate the typo of send: False for notification(Need confirmation), True for Indication (does not need confirmation) 
    *  @return Return the error code or a ESP_OK
    * */
esp_err_t ble_send_payload(unsigned char* payload, int payload_size, char status_id, char protocol_id, bool type){
    esp_err_t ret;
    
    ret = esp_ble_gatts_send_indicate(gl_profile_tab[PROFILE_APP_ID].gatts_if, 
                                        gl_profile_tab[PROFILE_APP_ID].conn_id, 
                                        gl_profile_tab[PROFILE_APP_ID].char_handle, 
                                        payload_size, payload, type);
    return ret;
}

void ble_send_full_payload(char status_id, char protocol_id, bool type) {
    const int msg_len = get_protocol_msg_length(protocol_id);
    unsigned char payload[msg_len];
    bzero(payload, msg_len);
    send_payload(payload, status_id, protocol_id, (unsigned short) ID_DEVICE, MAC_DEVICE);
    int to_send = msg_len;
    int offset = 0;
    while (to_send > 0) {
        int payload_size;
        if (to_send > 20) {
            payload_size = 20;
        } else {
            payload_size = to_send;
        }
        ESP_ERROR_CHECK(ble_send_payload(payload+offset, payload_size, status_id, protocol_id, false));
        offset = offset + payload_size;
        to_send = to_send - payload_size;
    }
}

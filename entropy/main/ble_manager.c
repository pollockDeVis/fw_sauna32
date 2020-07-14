/*
	File Name: ble_manager.c
	Author: Sadequr Rahman
	Date: 29/05/2020
	Description:
*/

#include "ble_manager.h"
#include "esp_log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "MessageQueue.h"
#include "IEEE11073float.h"

#define GAP_SERVICE_UUID   0x1800
#define GATTS_NUM_HANDLE   4

#define GAP_SERVICE_ID  0

static const char *TAG = __FILE__;

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

const char* advName = NULL;

uint16_t health_thermometer_handle_table[HT_IDX_NB];

uint8_t ble_system_activate_flag = false;
bool is_connect = false;
bool can_send_notify = false;
uint8_t notify_data[15];


//gap
esp_ble_adv_params_t* adv_params = NULL;


static uint8_t adv_service_uuid128[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0x09, 0x18, 0x00, 0x00,
};

//gatt
static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
					esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static  gatts_profile_inst_t health_thermometer_profile_tab[THERMOMETER_PROFILE_NUM] = {
    [THERMOMETER_PROFILE_APP_IDX] = {
        .gatts_cb = gatts_profile_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};


/* Service */
static const uint16_t GATTS_SERVICE_UUID      = 0x1809; //Health Thermometer Service ID
static const uint16_t GATTS_CHAR_UUID         = 0x2A1C; //Temperature Measurement Characteristic


static const uint16_t primary_service_uuid         = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid   = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t char_prop_read_write_notify   = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t health_thermometer_ccc[2]      = {0x00, 0x00};
static const uint8_t char_value[5]                 = {0x00, 0x00, 0x00, 0x0C, 0x84};

/* Full Database Description - Used to add attributes into the database */
static const esp_gatts_attr_db_t gatt_db[HT_IDX_NB] =
{
    // Service Declaration
    [HT_IDX_SVC]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID), (uint8_t *)&GATTS_SERVICE_UUID}},

    /* Characteristic Declaration */
    [HT_CHAR]     =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},

    /* Characteristic Value */
    [HT_CHAR_VAL] =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GATTS_DEMO_CHAR_VAL_LEN_MAX, sizeof(char_value), (uint8_t *)char_value}},

    /* Client Characteristic Configuration Descriptor */
    [HT_CHAR_CFG]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(uint16_t), sizeof(health_thermometer_ccc), (uint8_t *)health_thermometer_ccc}},


};

static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event) {

        case ESP_GATTS_REG_EVT:
        {
            esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HT_IDX_NB, SVC_INST_ID);
            if (create_attr_ret)
            {
                ESP_LOGE(TAG, "create attr table failed, error code = %x", create_attr_ret);
            }
        }
        break;
        case ESP_GATTS_READ_EVT:
            ESP_LOGI(TAG, "ESP_GATTS_READ_EVT");
       	    break;
        case ESP_GATTS_WRITE_EVT:
            if (!param->write.is_prep){
                // the data length of gattc write  must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
                //ESP_LOGI(TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
                esp_log_buffer_hex(TAG, param->write.value, param->write.len);
                uint16_t app_write = param->write.value[1]<<8 | param->write.value[0];
                if (app_write == THERMOMETER_APP_WRT_VAL)
                {
                	//ESP_LOGI(TAG, "JAILAM'S CODE!!");
                	ble_system_activate_flag = 1;
                	if(MessageQueue_IsValid()){
                	            	msg_t *m = (msg_t*) heap_caps_malloc(sizeof(msg_t), MALLOC_CAP_DEFAULT);
                	            	m->src = ble_system_activate;
                	            	m->msg = (void*)&ble_system_activate_flag;
                	            	MessageQueue_Send(m);
                	            	heap_caps_free(m);
                	            }
                }

                if (health_thermometer_handle_table[HT_CHAR_CFG] == param->write.handle && param->write.len == 2){
                    uint16_t descr_value = param->write.value[1]<<8 | param->write.value[0];
                    if (descr_value == 0x0001){
                        ESP_LOGI(TAG, "notify enable");
                        can_send_notify = true;

                        for (int i = 0; i < sizeof(notify_data); ++i) //#TODO check functionality and remove
                        {
                            notify_data[i] = i % 0xff;
                        }

                    }else if (descr_value == 0x0002){
                        ESP_LOGI(TAG, "indicate enable");
                        uint8_t indicate_data[15];
                        for (int i = 0; i < sizeof(indicate_data); ++i)
                        {
                            indicate_data[i] = i % 0xff;
                        }

                    }
                    else if (descr_value == 0x0000){
                    	can_send_notify =false;
                        ESP_LOGI(TAG, "notify/indicate disable ");
                    }

                    	else{

                        ESP_LOGE(TAG, "unknown descr value");
                        esp_log_buffer_hex(TAG, param->write.value, param->write.len);
                    }

                }
                /* send response when param->write.need_rsp is true*/
                if (param->write.need_rsp){
                    esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
                }
            }else{
                /* handle prepare write */
                example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
            }

      	    break;
        case ESP_GATTS_EXEC_WRITE_EVT:
            // the length of gattc prepare write data must be less than GATTS_DEMO_CHAR_VAL_LEN_MAX.
            ESP_LOGI(TAG, "ESP_GATTS_EXEC_WRITE_EVT");
            example_exec_write_event_env(&prepare_write_env, param);
            break;
        case ESP_GATTS_MTU_EVT:
           // ESP_LOGI(TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
            break;
        case ESP_GATTS_CONF_EVT:
           // ESP_LOGI(TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
            break;
        case ESP_GATTS_START_EVT:
           // ESP_LOGI(TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);
            break;

        case ESP_GATTS_CONNECT_EVT:
            is_connect = true;

            if(MessageQueue_IsValid()){
            	msg_t *m = (msg_t*) heap_caps_malloc(sizeof(msg_t), MALLOC_CAP_DEFAULT);
            	m->src = bluetooth;
            	m->msg = (void*)&is_connect;
            	MessageQueue_Send(m);
            	heap_caps_free(m);
            }

            esp_ble_conn_update_params_t conn_params = {0};
            memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            /* For the iOS system, please refer to Apple official documents about the BLE connection parameters restrictions. */
            conn_params.latency = 0;
            conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
            conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
            conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
            //start sent the update connection parameters to the peer device.
            ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                        param->connect.conn_id,
                        param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                        param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
            health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].conn_id = param->connect.conn_id;
            //start sent the update connection parameters to the peer device.
            esp_ble_gap_update_conn_params(&conn_params);
			break;

         case ESP_GATTS_DISCONNECT_EVT:
            is_connect = false;

            if(MessageQueue_IsValid()){
            	msg_t *m = (msg_t*) heap_caps_malloc(sizeof(msg_t), MALLOC_CAP_DEFAULT);
            	m->src = bluetooth;
            	m->msg = (void*)&is_connect;
            	MessageQueue_Send(m);
            	heap_caps_free(m);
            }
                ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);
                esp_ble_gap_start_advertising(adv_params);
            break;
        case ESP_GATTS_CREAT_ATTR_TAB_EVT:{
            if (param->add_attr_tab.status != ESP_GATT_OK){
                ESP_LOGE(TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
            }
            else if (param->add_attr_tab.num_handle != HT_IDX_NB){
                ESP_LOGE(TAG, "create attribute table abnormally, num_handle (%d) \
                        doesn't equal to HTS_IDX_NB(%d)", param->add_attr_tab.num_handle, HT_IDX_NB);
            }
            else {
                ESP_LOGI(TAG, "create attribute table successfully, the number handle = %d\n",param->add_attr_tab.num_handle);
                memcpy(health_thermometer_handle_table, param->add_attr_tab.handles, sizeof(health_thermometer_handle_table));
                esp_ble_gatts_start_service(health_thermometer_handle_table[HT_IDX_SVC]);
            }

        }
        break;
        case ESP_GATTS_STOP_EVT:
        case ESP_GATTS_OPEN_EVT:
        case ESP_GATTS_CANCEL_OPEN_EVT:
        case ESP_GATTS_CLOSE_EVT:
        case ESP_GATTS_LISTEN_EVT:
        case ESP_GATTS_CONGEST_EVT:
        case ESP_GATTS_UNREG_EVT:
        case ESP_GATTS_DELETE_EVT:
        default:
            break;
    }
}

static void sensor_data_callback(void *msg) {
	//ESP_LOGI(TAG, "FLOAT Val : %lf\r\n", *((float* )msg));

	uint8_t temp_measurement[BLE_TEMP_BUFF_SIZE - 1] = { '\0' }; //Tempearature is in Centigrade unit
	uint8_t BLE_measurement[BLE_TEMP_BUFF_SIZE] = { '\0' }; //Tempearature is in Centigrade unit
	BLE_measurement[0] = 0x00; //Flag

	float2IEEE11073(*((float*) msg), temp_measurement);

	for (uint8_t i = 0; i < (BLE_TEMP_BUFF_SIZE - 1); i++)
		BLE_measurement[i + 1] = temp_measurement[i];
    if(ble_system_activate_flag)
    	ble_manager_send_indication(BLE_measurement, BLE_TEMP_BUFF_SIZE);
}

void ble_manager_init()
{
    esp_err_t ret;
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Bluetooth initializing done.");
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "Bluetooth mode set to Low energy only");
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "bluedroid initialized");
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ESP_LOGI(TAG, "bluedroid enabled");
    ret = esp_ble_gatts_register_callback(gatts_event_handler); //changed -palok
    if (ret){
        ESP_LOGE(TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ESP_LOGI(TAG, "GATT event handler registered");
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        ESP_LOGE(TAG, "gap register error, error code = %x", ret);
        return;
    }
    ESP_LOGI(TAG, "GAP event handler registered");
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(512);
    if (local_mtu_ret){
        ESP_LOGE(TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }
    ESP_LOGI(TAG, "Local mtu set to 512");
    ret = esp_ble_gatts_app_register(GAP_SERVICE_ID);
    if (ret){
        ESP_LOGE(TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    ESP_LOGI(TAG, "Register to gap service");

    ble_manager_setDeviceName(BLE_DEVICE_NAME);
    	esp_ble_adv_data_t *advData = ble_manager_getDefaultAdvData();
    	esp_ble_adv_params_t *advParams = ble_manager_getDefaultAdvertiseParam();
    	esp_ble_adv_data_t *scanResp = ble_manager_getDefaultAdvRespData();

    	ble_manager_setAdvParams(advParams);
    	ble_manager_setAdvRespData(scanResp);
    	ble_manager_setAdvData(advData);

	MessageQueue_RegisterMsg(sensor, sensor_data_callback);
}

esp_ble_adv_params_t* ble_manager_getDefaultAdvertiseParam(void)
{
    esp_ble_adv_params_t * default_adv_params = (esp_ble_adv_params_t*) malloc(sizeof(esp_ble_adv_params_t));
    memset((void*)default_adv_params,0,sizeof(esp_ble_adv_params_t));
    default_adv_params->adv_int_min        = 0x20;
    default_adv_params->adv_int_max        = 0x40;
    default_adv_params->adv_type           = ADV_TYPE_IND;
    default_adv_params->own_addr_type      = BLE_ADDR_TYPE_PUBLIC;
    default_adv_params->channel_map        = ADV_CHNL_ALL;
    default_adv_params->adv_filter_policy  = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY;
    return default_adv_params;
}

esp_ble_adv_data_t* ble_manager_getDefaultAdvRespData(void)
{
    esp_ble_adv_data_t *default_scan_rsp_data = (esp_ble_adv_data_t*) malloc(sizeof(esp_ble_adv_data_t));
    memset((void*)default_scan_rsp_data,0,sizeof(esp_ble_adv_data_t));
    default_scan_rsp_data->set_scan_rsp = true;
    default_scan_rsp_data->include_name = true;
    default_scan_rsp_data->include_txpower = true;
    default_scan_rsp_data->appearance = 0x00;
    default_scan_rsp_data->manufacturer_len = 0; //TEST_MANUFACTURER_DATA_LEN,
    default_scan_rsp_data->p_manufacturer_data =  NULL; //&test_manufacturer[0],
    default_scan_rsp_data->service_data_len = 0;
    default_scan_rsp_data->p_service_data = NULL;
    default_scan_rsp_data->service_uuid_len = sizeof(adv_service_uuid128);
    default_scan_rsp_data->p_service_uuid = adv_service_uuid128;
    default_scan_rsp_data->flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

    return default_scan_rsp_data;
}

esp_ble_adv_data_t* ble_manager_getDefaultAdvData(void)
{
    esp_ble_adv_data_t* default_adv_data = (esp_ble_adv_data_t*) malloc(sizeof(esp_ble_adv_data_t));
    memset((void*)default_adv_data,0,sizeof(esp_ble_adv_data_t));
    default_adv_data->set_scan_rsp = false;
    default_adv_data->include_name = true;
    default_adv_data->include_txpower = true;
    default_adv_data->min_interval = 0x0006;
    default_adv_data->max_interval = 0x0010;
    default_adv_data->appearance = 0x00;
    default_adv_data->manufacturer_len = 0; //TEST_MANUFACTURER_DATA_LEN,
    default_adv_data->p_manufacturer_data =  NULL; //&test_manufacturer[0],
    default_adv_data->service_data_len = 0;
    default_adv_data->p_service_data = NULL;
    default_adv_data->service_uuid_len = 32;
    default_adv_data->p_service_uuid = adv_service_uuid128;
    default_adv_data->flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT);

    return default_adv_data;
}

esp_err_t ble_manager_setDeviceName(const char* deviceName)
{
    advName = deviceName;
    esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(deviceName);
    if (set_dev_name_ret){
        ESP_LOGE(TAG, "set device name failed, error code = %x", set_dev_name_ret);
    }
    return set_dev_name_ret;
}

const char* ble_manager_getDeviceName(void)
{
    return advName;
}

void ble_manager_setAdvParams(esp_ble_adv_params_t* params)
{
    adv_params = params;
}

esp_err_t ble_manager_setAdvData(esp_ble_adv_data_t* advData)
{
    esp_err_t ret = esp_ble_gap_config_adv_data(advData);
    if (ret){
        ESP_LOGE(TAG, "config adv data failed, error code = %x", ret);
    }
    return ret;
}

esp_err_t ble_manager_setAdvRespData(esp_ble_adv_data_t* scanRespData)
{
    esp_err_t ret = esp_ble_gap_config_adv_data(scanRespData);
    if (ret){
        ESP_LOGE(TAG, "config scan response data failed, error code = %x", ret);
    }
    return ret;
}

void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        if(adv_params != NULL)
            esp_ble_gap_start_advertising(adv_params);
        else
            ESP_LOGI(TAG, "Ble advParams is null");
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        ESP_LOGD(TAG, "ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT");
        if(adv_params != NULL)
            esp_ble_gap_start_advertising(adv_params);
        else
            ESP_LOGI(TAG, "Ble advParams is null");
        break;

    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising start failed\n");
        }else{
           ESP_LOGI(TAG, "Ble Advertising");
        }

        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising stop failed\n");
        } else {
            ESP_LOGI(TAG, "Stop adv successfully\n");
        }
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
            param->update_conn_params.status,
            param->update_conn_params.min_int,
            param->update_conn_params.max_int,
            param->update_conn_params.conn_int,
            param->update_conn_params.latency,
            param->update_conn_params.timeout);
        break;
    case ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT:
        ESP_LOGD(TAG, "ESP_GAP_BLE_GET_BOND_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT:
        ESP_LOGD(TAG, "ESP_GAP_BLE_CLEAR_BOND_DEV_COMPLETE_EVT");
        break;
    case ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT:
        ESP_LOGD(TAG, "ESP_GAP_BLE_REMOVE_BOND_DEV_COMPLETE_EVT");
        break;
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{

    /* If event is register event, store the gatts_if for each profile */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK)
        {
            health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].gatts_if = gatts_if;
                } else {
                    ESP_LOGE(TAG, "reg app failed, app_id %04x, status %d",
                            param->reg.app_id,
                            param->reg.status);
                    return;
        }
    }
    do {

    /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
    if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
    gatts_if == health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].gatts_if) 
    {
        if (health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].gatts_cb) {
            health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].gatts_cb(event, gatts_if, param);
        }
    }

    } while (0);
}

void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;
    if (prepare_write_env->prepare_buf == NULL) {
        prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL) {
            ESP_LOGE(TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    } else {
        if(param->write.offset > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_OFFSET;
        } else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE) {
            status = ESP_GATT_INVALID_ATTR_LEN;
        }
    }
    /*send response when param->write.need_rsp is true */
    if (param->write.need_rsp){
        esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL){
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;
            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp);
            if (response_err != ESP_OK){
               ESP_LOGE(TAG, "Send response error");
            }
            free(gatt_rsp);
        }else{
            ESP_LOGE(TAG, "%s, malloc failed", __func__);
        }
    }
    if (status != ESP_GATT_OK){
        return;
    }
    memcpy(prepare_write_env->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;

}

void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param){
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf){
        esp_log_buffer_hex(TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }else{
        ESP_LOGI(TAG,"ESP_GATT_PREP_WRITE_CANCEL");
    }
    if (prepare_write_env->prepare_buf) {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

void ble_manager_send_indication(uint8_t* buf, uint8_t len)
{
    if(is_connect && can_send_notify)
    {
        esp_ble_gatts_send_indicate(health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].gatts_if, 
                                    health_thermometer_profile_tab[THERMOMETER_PROFILE_APP_IDX].conn_id, 
                                    health_thermometer_handle_table[HT_CHAR_VAL], 
                                    len, 
                                    buf,
                                    true);
    }
}

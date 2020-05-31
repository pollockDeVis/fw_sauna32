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


#define GAP_SERVICE_ID  0

static const char *TAG = __FILE__;
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
const char* advName = NULL;


//gap
esp_ble_adv_params_t* adv_params = NULL; 
static uint8_t adv_service_uuid128[32] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    //first uuid, 16bit, [12],[13] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xEE, 0x00, 0x00, 0x00,
    //second uuid, 32bit, [12], [13], [14], [15] is the value
    0xfb, 0x34, 0x9b, 0x5f, 0x80, 0x00, 0x00, 0x80, 0x00, 0x10, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00,
};



//gatt


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
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
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
#ifdef CONFIG_SET_RAW_ADV_DATA
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        break;
#else
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
#endif
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


void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_CONNECT_EVT: {
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        ESP_LOGI(TAG, "ESP_GATTS_CONNECT_EVT, conn_id %d, remote %02x:%02x:%02x:%02x:%02x:%02x:",
                 param->connect.conn_id,
                 param->connect.remote_bda[0], param->connect.remote_bda[1], param->connect.remote_bda[2],
                 param->connect.remote_bda[3], param->connect.remote_bda[4], param->connect.remote_bda[5]);
        //start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:
        ESP_LOGI(TAG, "ESP_GATTS_DISCONNECT_EVT, disconnect reason 0x%x", param->disconnect.reason);
        esp_ble_gap_start_advertising(adv_params);
        break;
    default:
        break;
    }

}
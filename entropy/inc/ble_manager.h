/*
	File Name: ble_manager.h
	Author: Sadequr Rahman
	Date: 29/05/2020
	Description:
*/

#ifndef _BLE_MANAGER_H_
#define _BLE_MANAGER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

//GATT Application Profiles
#define THERMOMETER_PROFILE_NUM					1
#define THERMOMETER_PROFILE_APP_IDX				0
#define THERMOMETER_APP_ID						0x55 //USER DEFINED
#define SVC_INST_ID                 			0

#define GATTS_DEMO_CHAR_VAL_LEN_MAX 			500
#define PREPARE_BUF_MAX_SIZE        			1024
#define CHAR_DECLARATION_SIZE       			(sizeof(uint8_t))

//GATT Profile

enum
{
	HT_IDX_SVC,
	HT_CHAR,
	HT_CHAR_VAL,
	HT_CHAR_CFG,
	HT_IDX_NB,
};

typedef struct {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;

}gatts_profile_inst_t;

typedef struct {
    uint8_t                 *prepare_buf;
    int                     prepare_len;
} prepare_type_env_t;

prepare_type_env_t prepare_write_env;

void ble_manager_init(void);
esp_err_t ble_manager_setDeviceName(const char* deviceName);
const char* ble_manager_getDeviceName(void);

// gap
esp_ble_adv_data_t * ble_manager_getDefaultAdvData(void);
esp_ble_adv_params_t * ble_manager_getDefaultAdvertiseParam(void);
esp_ble_adv_data_t * ble_manager_getDefaultAdvRespData(void);
void ble_manager_setAdvParams(esp_ble_adv_params_t* param);
esp_err_t ble_manager_setAdvData(esp_ble_adv_data_t* advData);
esp_err_t ble_manager_setAdvRespData(esp_ble_adv_data_t* scanRespData);
//gatt
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);
void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param);

#endif /* _BLE_MANAGER_H_ */

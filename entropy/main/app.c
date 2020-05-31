/*
	File Name: app.c
	Author: Sadequr Rahman
	Date: 29/05/2020

	Description: 
*/
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "heartBeat.h"
#include "ble_manager.h"



void app_main()
{
    esp_err_t ret;
    
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	
	ble_manager_init();
	ble_manager_setDeviceName("Rabby-Esp32");
	esp_ble_adv_data_t *advData = ble_manager_getDefaultAdvData();
	esp_ble_adv_params_t *advParams = ble_manager_getDefaultAdvertiseParam();
	esp_ble_adv_data_t *scanResp = ble_manager_getDefaultAdvRespData();


	ble_manager_setAdvParams(advParams);
	ble_manager_setAdvRespData(scanResp);
	ble_manager_setAdvData(advData);
	heartBeat_init();

}

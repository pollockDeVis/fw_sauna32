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


static const char *TAG = __FILE__;


void app_main()
{
    esp_err_t ret;
    
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	heartBeat_init();

}




#include "display_driver.h"
#include "temp_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_manager.h"
#include "sensor_filter.h"
#include "buzzer.h"
#include "IEEE11073float.h"



static const char *TAG = __FILE__;


void app_main()
{
    esp_err_t ret;

    /* Initialize NVS. */
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	temp_driver_init();
	display_driver_init();
	display_start_page();

	ble_manager_init();
	ble_manager_setDeviceName("SAUNA32 Lite");
	esp_ble_adv_data_t *advData = ble_manager_getDefaultAdvData();
	esp_ble_adv_params_t *advParams = ble_manager_getDefaultAdvertiseParam();
	esp_ble_adv_data_t *scanResp = ble_manager_getDefaultAdvRespData();

	ble_manager_setAdvParams(advParams);
	ble_manager_setAdvRespData(scanResp);
	ble_manager_setAdvData(advData);

	  while (1)
	    {
		  ESP_LOGI(TAG, "This is a test");
		  vTaskDelay(1000 / portTICK_RATE_MS);
	    }

}




#include "display_driver.h"
#include "temp_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_manager.h"
#include "sensor_filter.h"

static const char *TAG = "app";


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
    ble_manager_init();
	temp_driver_init();

	display_driver_init();
	float to = 0, to2 = 0, ta = 0;
	ringBuffParams *tmp;
	tmp = sensor_filter_init(100);
	for(uint8_t i = 0 ; i <= 100; ++i)
	{
		sensor_filter_put_raw_data(tmp, temp_driver_get_obj_temp());
		ESP_LOGI("TAG", "%d \r\n", i) ;
	}
	float avg = sensor_filter_get_filtered_data(tmp);
	ESP_LOGI("TAG", "Average is %lf \r\n", avg) ;

	  while (1)
	    {
			to = temp_driver_get_obj_temp();
			ta = temp_driver_get_amb_temp();
			to2 = temp_driver_get_obj_2_temp();
		  ESP_LOGI("TAG", "log:%lf %lf %lf \r\n", to, to2, ta) ;
		  display_temperature(to);
		  vTaskDelay(100 / portTICK_RATE_MS);
	    }

}

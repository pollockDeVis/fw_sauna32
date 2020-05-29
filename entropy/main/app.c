


#include "display_driver.h"
#include "temp_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_manager.h"
#include "sensor_filter.h"

#define FEVER_TEMPERATURE_THRESHOLD     37.3

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
	display_start_page();
	vTaskDelay(2000 / portTICK_RATE_MS);
	float to = 0, to2 = 0, ta = 0;
	//float last_avg = 0;
	uint8_t sample_size = 100;

	//			ringBuffParams *tmp;
	//			tmp = sensor_filter_init(sample_size);
	//
	//			for(uint8_t i = 0 ; i <= sample_size; ++i)
	//			{
	//				to = temp_driver_get_obj_temp();
	//				sensor_filter_put_raw_data(tmp, to);
	//				//ESP_LOGI("TAG", "%d \r\n", i) ;
	//				vTaskDelay(10 / portTICK_RATE_MS);
	//			}
	//			float avg = sensor_filter_get_filtered_data(tmp);
	//			ESP_LOGI("TAG", "Average is %lf \r\n", avg) ;
	//			ESP_LOGI("TAG", "Gradient is %lf \r\n", avg-last_avg ) ;
	//			last_avg = avg;
	  to = temp_driver_get_obj_temp();
	  display_temperature(to, FEVER_TEMPERATURE_THRESHOLD);
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_refresh();
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_driver_error_header("BLE DISCONNECTED", "ERROR");
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_refresh();
	  display_generic_message();
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_refresh();

	  while (1)
	    {
			to = temp_driver_get_obj_temp();
			ta = temp_driver_get_amb_temp();
			to2 = temp_driver_get_obj_2_temp();
		  ESP_LOGI("TAG", "log:%lf %lf %lf \r\n", to, to2, ta) ;
		  display_temperature(to, FEVER_TEMPERATURE_THRESHOLD);
		  vTaskDelay(100 / portTICK_RATE_MS);
	    }

}

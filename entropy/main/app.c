


#include "display_driver.h"
#include "temp_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_manager.h"

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

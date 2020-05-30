


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

	  while (1)
	    {
		  vTaskDelay(1000 / portTICK_RATE_MS);
	    }

}

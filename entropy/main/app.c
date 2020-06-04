



#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "ble_manager.h"
#include "sensor_manager.h"
#include "display_driver.h"
#include "MessageQueue.h"

#include <stdlib.h>
#include <string.h>


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
	MessageQueue_Init(MSG_QUEUE_LEN);
	display_driver_init();
	ble_manager_init();
    sensor_manager_init(); //maybe should be after BLE


}

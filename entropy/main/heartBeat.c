/*
 * heartBeat.c
 *
 *  Created on: 13 Nov 2019
 *      Author: rabby
 */

#include "heartBeat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = __FILE__;

static void heartBeat_task(void *args) {
	ESP_LOGI(TAG, "starting heartbeat");
	gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
	int level = 0;
	while (true) {
		gpio_set_level(GPIO_NUM_2, level);
		level = !level;
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
	ESP_LOGI(TAG, "Stop heart beat");
}

void heartBeat_init(void) {

	xTaskCreate(heartBeat_task, "heartBeat", 1024 * 2, NULL, 5, NULL);

}


/*
 * led.c
 *
 *  Created on: Jul 16, 2020
 *      Author: Pollock
 */

#include "led.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t ledBlinkTaskHandle = NULL;

static void led_setup()
{
	gpio_pad_select_gpio(LED_PIN);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

static void blink(void *pvParameter)
{
	while(1)
	{
	  gpio_set_level(LED_PIN, 0);
	  vTaskDelay(500 / portTICK_PERIOD_MS);
	  /* Blink on (output high) */
	  gpio_set_level(LED_PIN, 1);
	  vTaskDelay(100 / portTICK_PERIOD_MS);

	}
}

void led_off()
{
	gpio_set_level(LED_PIN, 0);
	vTaskSuspend(ledBlinkTaskHandle);
}
void led_on()
{
	vTaskResume(ledBlinkTaskHandle);
}

void led_init()
{
	led_setup();
	xTaskCreate(&blink, "blink on", configMINIMAL_STACK_SIZE, NULL, 5, &ledBlinkTaskHandle);
}




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
#include "MessageQueue.h"

TaskHandle_t ledBlinkTaskHandle = NULL;

static void ble_status_callback(void *msg) {
	if (*((bool* )msg) == true)
	{
		led_blink_off();
		led_on();
	}
	else if (*((bool* )msg) == false)
	{
		led_blink_on();
	}
}

static void led_setup()
{
	gpio_pad_select_gpio(LED_PIN);
	/* Set the GPIO as a push/pull output */
	gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT);
}

void led_on()
{
	gpio_set_level(LED_PIN, 1);
}
void led_off()
{
	gpio_set_level(LED_PIN, 0);
}

static void blink(void *pvParameter)
{
	while(1)
	{
	  led_off();
	  vTaskDelay(500 / portTICK_PERIOD_MS);
	  /* Blink on (output high) */
	  led_on();
	  vTaskDelay(100 / portTICK_PERIOD_MS);

	}
}

void led_blink_off()
{
	led_off();
	vTaskSuspend(ledBlinkTaskHandle);
}
void led_blink_on()
{
	vTaskResume(ledBlinkTaskHandle);
}

void led_init()
{
	led_setup();
	MessageQueue_RegisterMsg(bluetooth, ble_status_callback);
	xTaskCreate(&blink, "blink on", configMINIMAL_STACK_SIZE, NULL, 5, &ledBlinkTaskHandle);
	led_blink_on();
}




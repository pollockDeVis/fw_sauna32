/*
 * buzzer.c
 *
 *  Created on: May 29, 2020
 *      Author: Pollock
 */

#include "buzzer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "driver/ledc.h"

TaskHandle_t buzzerTaskHandle = NULL;
ledc_channel_config_t buzzer_channel;

//callback
//vTaskResume(ledBlinkTaskHandle);
//vTaskSuspend(ledBlinkTaskHandle);

static void buzzer_on(void *pv)
{
	while(1)
	{
        ledc_set_duty(buzzer_channel.speed_mode, buzzer_channel.channel, BUZZER_DUTY_CYCLE);
        ledc_update_duty(buzzer_channel.speed_mode, buzzer_channel.channel);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        ledc_set_duty(buzzer_channel.speed_mode, buzzer_channel.channel, 0);
        ledc_update_duty(buzzer_channel.speed_mode, buzzer_channel.channel);
        vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

void buzzer_init()
{
    ledc_timer_config_t buzzer_timer = {
        .duty_resolution = LEDC_TIMER_13_BIT, // resolution of PWM duty
        .freq_hz = BUZZER_FREQUENCY,                      // frequency of PWM signal
        .speed_mode = BUZZER_HS_MODE,           // timer mode
        .timer_num = BUZZER_HS_TIMER,            // timer index
        .clk_cfg = LEDC_AUTO_CLK,              // Auto select the source clock
    };
    // Set configuration of timer0 for high speed channels
    ledc_timer_config(&buzzer_timer);

    ledc_channel_config_t buzzer_channel = {

            .channel    = BUZZER_HS_CH0_CHANNEL,
            .duty       = 0,
            .gpio_num   = BUZZER_HS_CH0_GPIO,
            .speed_mode = BUZZER_HS_MODE,
            .hpoint     = 0,
            .timer_sel  = BUZZER_HS_TIMER


    };

    ledc_channel_config(&buzzer_channel);
    xTaskCreate(&buzzer_on, "buzzer_on", configMINIMAL_STACK_SIZE, NULL, 5, &buzzerTaskHandle);
}

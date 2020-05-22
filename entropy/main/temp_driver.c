/*
 * temp_driver.c
 *
 *  Created on: May 22, 2020
 *      Author: Pollock
 */
#include <stdio.h>
#include "temp_driver.h"
#include "MLX90614_API.h"
#include "MLX90614_SMBus_Driver.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void temp_driver_init()
{


	    MLX90614_SMBusInit(MLX90614_SDA_GPIO,  MLX90614_SCL_GPIO, 50000); // sda scl and 50kHz
	    vTaskDelay(1000/portTICK_RATE_MS);
}
float temp_driver_get_obj_temp()
{
	float _to = 0;
	MLX90614_GetTo(MLX90614_DEFAULT_ADDRESS, &_to);
	return _to;

}
float temp_driver_get_amb_temp()
{
	float _ta = 0;
	MLX90614_GetTa(MLX90614_DEFAULT_ADDRESS, &_ta);
	return _ta;
}

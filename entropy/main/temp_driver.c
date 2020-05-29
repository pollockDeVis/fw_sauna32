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
#include "esp_log.h"

static const char *TAG = "temp_driver";

void temp_driver_init()
{


	    MLX90614_SMBusInit(MLX90614_SDA_GPIO,  MLX90614_SCL_GPIO, MLX90614_I2C_CLOCK_FREQUENCY); // sda scl and 50kHz
	    vTaskDelay(100/portTICK_RATE_MS);
		float emissivity;
		MLX90614_GetEmissivity(MLX90614_DEFAULT_ADDRESS, &emissivity);
		ESP_LOGI(TAG, "Emissivity is %lf \r\n", emissivity) ;
/*
		emissivity = 0.95;
		int error = MLX90614_SetEmissivity(MLX90614_DEFAULT_ADDRESS, emissivity);
		ESP_LOGI(TAG, "Error %d", error);
		MLX90614_GetEmissivity(MLX90614_DEFAULT_ADDRESS, &emissivity);
		ESP_LOGI(TAG, "After set Emissivity is %lf \r\n", emissivity) ;
*/
}
float temp_driver_get_obj_temp()
{
	float _to = 0;
	MLX90614_GetTo(MLX90614_DEFAULT_ADDRESS, &_to);
	if(_to < MINIMUM_MEASURED_TEMP) _to = MINIMUM_MEASURED_TEMP;
	else if (_to > MAXIMUM_MEASURED_TEMP) _to = MAXIMUM_MEASURED_TEMP;

	return _to;

}

float temp_driver_get_obj_2_temp()
{
	float _to2 = 0;
	MLX90614_GetTo2(MLX90614_DEFAULT_ADDRESS, &_to2);
	return _to2;
}

float temp_driver_get_amb_temp()
{
	float _ta = 0;
	MLX90614_GetTa(MLX90614_DEFAULT_ADDRESS, &_ta);
	return _ta;
}

/*
 * sensor_manager.c
 *
 *  Created on: May 29, 2020
 *      Author: Pollock
 */
#include "sensor_manager.h"
#include "esp_log.h"

TaskHandle_t sensorManagerDataHandle = NULL;

float filtered_temp_float = 0.0; //static make it and test
uint8_t filtered_temp_IEEE11073[IEEE_TEMP_BUFF_LEN] = { '\0' }; //static


const char* TAG = __FILE__;

static void temp_data_filter_task(void *param)
{

	ringBuffParams *tmp;
	tmp = sensor_filter_init(SENSOR_SAMPLE_SIZE);
	uint16_t sample_count = 0;

	while(1)
	{
		sensor_filter_put_raw_data(tmp, temp_driver_get_obj_temp());

		if(sample_count > SENSOR_SAMPLE_SIZE)
		{
			filtered_temp_float = sensor_filter_get_filtered_data(tmp); //Update the temperature static variable
			uint8_t temp_measurement[4] = { '\0' };
			float2IEEE11073(filtered_temp_float, temp_measurement); //maybe can be done on the BLE end since it is util
			for (uint8_t i = 0; i < 4; i++) filtered_temp_IEEE11073[i+1] = temp_measurement[i];
			//Trigger Events -> BLE & Display (optional-> need to check)
			ESP_LOGI("TAG", "filtered_temp_float is %lf \r\n", filtered_temp_float) ;
			sample_count = 0;
		}

		sample_count++;
		vTaskDelay(SENSOR_SAMPLING_TIME_MS / portTICK_RATE_MS);
	}
}

void sensor_manager_init()
{
	temp_driver_init();
	filtered_temp_IEEE11073[0] = 0x00; //Flag byte for the Encoding
	xTaskCreate(temp_data_filter_task, "Data Acquisition Task", 1024*4, NULL, configMAX_PRIORITIES-3, &sensorManagerDataHandle);
}



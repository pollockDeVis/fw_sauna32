/*
 * sensor_manager.c
 *
 *  Created on: May 29, 2020
 *      Author: Pollock
 */
#include "sensor_manager.h"
#include "esp_log.h"
#include "MessageQueue.h"

TaskHandle_t sensorManagerDataHandle = NULL;

static const char *TAG = __FILE__;

//Equation Params
float c = 53.7;
float b = -2.36;
float a = 0.025;
float Tc_offset = 0.0;
float static_distance_comp = 2.985;
//Equation Params

float filtered_temp_float = 0.0; //static make it and test
uint8_t filtered_temp_IEEE11073[IEEE_TEMP_BUFF_LEN] = { '\0' }; //static




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
			filtered_temp_float = sensor_filter_get_filtered_data(tmp) +  static_distance_comp; //Update the temperature static variable
			//ESP_LOGI(TAG, "filtered_temp_float is %lf \r\n", filtered_temp_float) ;
			if(filtered_temp_float >= BODY_TEMPERATURE_MIN && filtered_temp_float <= BODY_TEMPERATURE_MAX)
				Tc_offset = c + b*filtered_temp_float + a*(filtered_temp_float*filtered_temp_float);
			else
				Tc_offset = 0.0;
			filtered_temp_float = filtered_temp_float + Tc_offset;
            if(MessageQueue_IsValid()){
            	msg_t *m = (msg_t*) heap_caps_malloc(sizeof(msg_t), MALLOC_CAP_DEFAULT);
            	m->src = sensor;
            	m->msg = (void*)&filtered_temp_float;
            	MessageQueue_Send(m);
            	heap_caps_free(m);
            }

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



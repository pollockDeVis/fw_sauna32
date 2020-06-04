/*
 * sensor_manager.h
 *
 *  Created on: May 29, 2020
 *      Author: Pollock
 */

#ifndef INC_SENSOR_MANAGER_H_
#define INC_SENSOR_MANAGER_H_

#include "temp_driver.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sensor_filter.h"

#define SENSOR_SAMPLE_SIZE			50
#define SENSOR_SAMPLING_TIME_MS		5
#define IEEE_TEMP_BUFF_LEN			5
void sensor_manager_init(void);


#endif /* INC_SENSOR_MANAGER_H_ */

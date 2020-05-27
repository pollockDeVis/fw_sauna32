/*
 * sensor_filter.h
 *
 *  Created on: May 28, 2020
 *      Author: Pollock
 */

#ifndef INC_SENSOR_FILTER_H_
#define INC_SENSOR_FILTER_H_

#include <stdlib.h>
#include <stdio.h>


typedef struct ring_buffer_params
{
	uint8_t size;
	float 	*buff;
}ringBuffParams;

ringBuffParams *sensor_filter_init(uint8_t _size);
void sensor_filter_delete_ring_buff(ringBuffParams *_rb);
float sensor_filter_get_filtered_data(ringBuffParams *_rb);
void sensor_filter_put_raw_data(ringBuffParams *_rb, float _data);

#endif /* INC_SENSOR_FILTER_H_ */

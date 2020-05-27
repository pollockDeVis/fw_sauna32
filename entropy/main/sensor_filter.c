/*
 * sensor_filter.c
 *
 *  Created on: May 28, 2020
 *      Author: Pollock
 */

#include "sensor_filter.h"
#include <stdint.h>
#include <string.h>

ringBuffParams *sensor_filter_init(uint8_t _size)
{
	ringBuffParams *_sensor = NULL; //creating a NULL pointer
	_sensor = (ringBuffParams*)malloc(sizeof(ringBuffParams));
	_sensor->size = _size;
	_sensor->buff = NULL;
	_sensor->buff = (float*)malloc(sizeof(float)*_sensor->size); //allocating dynamic memory for the specified size of ring buff
	memset((void*) _sensor->buff, 0, sizeof(float)*_sensor->size);
	return _sensor;

}
void sensor_filter_delete_ring_buff(ringBuffParams *_rb)
{
	if(_rb) //checking if the pointer is NULL
	{
		free(_rb->buff);
		free(_rb);
	}
}

void sensor_filter_put_raw_data(ringBuffParams *_rb, float _data)
{
	if(_rb)
	{
		for(uint8_t i = (_rb->size -1); i > 0; --i)
		{
			_rb->buff[i] = _rb->buff[i-1]; //shifting all values by 1 step

		}
		_rb->buff[0] = _data; //new data always gets placed in the first position
	}
}

float sensor_filter_get_filtered_data(ringBuffParams *_rb)
{
	float _avg = 0;
	if(_rb)
	{
		for(uint8_t i=0; i <_rb->size; ++i)
		{
			_avg += _rb->buff[i];
		}
		_avg = _avg/_rb->size;
	}
	return _avg;
}



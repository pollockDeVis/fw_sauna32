/*
 * temp_driver.h
 *
 *  Created on: May 22, 2020
 *      Author: Pollock
 */

#ifndef INC_TEMP_DRIVER_H_
#define INC_TEMP_DRIVER_H_

#define MLX90614_DEFAULT_ADDRESS 		0x5A // default chip address(slave address) of MLX90614

#define MLX90614_I2C_CLOCK_FREQUENCY	100000
#define MLX90614_SDA_GPIO 				21 // sda for MLX90614
#define MLX90614_SCL_GPIO 				22 // scl for MLX90614
#define MINIMUM_MEASURED_TEMP			10
#define MAXIMUM_MEASURED_TEMP			99

void temp_driver_init();
float temp_driver_get_obj_temp();
float temp_driver_get_obj_2_temp();
float temp_driver_get_amb_temp();
//factory emissivity 0.949996
#endif /* INC_TEMP_DRIVER_H_ */

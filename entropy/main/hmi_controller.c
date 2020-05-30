/*
 * hmi_controller.c
 *
 *  Created on: May 29, 2020
 *      Author: Pollock
 */
#define FEVER_TEMPERATURE_THRESHOLD     37.3

/*
 * 	display_start_page();
	vTaskDelay(2000 / portTICK_RATE_MS);
	float to = 0, to2 = 0, ta = 0;

	  to = temp_driver_get_obj_temp();
	  display_temperature(to, FEVER_TEMPERATURE_THRESHOLD);
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_refresh();
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_driver_error_header("BLE DISCONNECTED", "ERROR");
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_refresh();
	  display_generic_message();
	  vTaskDelay(2000 / portTICK_RATE_MS);
	  display_refresh();
	  buzzer_init();

	  			to = temp_driver_get_obj_temp();
			ta = temp_driver_get_amb_temp();
			to2 = temp_driver_get_obj_2_temp();
		  ESP_LOGI("TAG", "log:%lf %lf %lf \r\n", to, to2, ta) ;
		  display_temperature(to, FEVER_TEMPERATURE_THRESHOLD);
		  vTaskDelay(100 / portTICK_RATE_MS);
 */


/*
 * display_manager.h
 *
 *  Created on: May 22, 2020
 *      Author: Pollock
 *      Company: Digital Forest Sdn Bhd
 */

#ifndef INC_DISPLAY_DRIVER_H_
#define INC_DISPLAY_DRIVER_H_

// ==========================================================

#define SPI_BUS TFT_HSPI_HOST		// Define which spi bus to use TFT_VSPI_HOST or TFT_HSPI_HOST
#define SPI_MAX_CLOCK_FREQ			8000000
#define SPI_MAX_DATA_TRANSFER_SIZE  6*1024
// ==========================================================
#define FEVER_TEMPERATURE_THRESHOLD     37.3
#define DISPLAY_BLE_DISCONNECTED_MSG	"BLE DISCONNECTED"
#define DISPLAY_BLE_CONNECTED_MSG		"BLE CONNECTED"
#define DISPLAY_ERROR_MSG				"ERROR!"
#define DISPLAY_DEVICE_NAME    			"SAUNA32 Lite"
#define	DISPLAY_POWERED_BY				"Powered by"
#define	DISPLAY_DF						"Digital Forest"

void display_driver_default_header(char *info);
void display_driver_error_header(char *hdr, char *ftr);
void display_driver_init();
void display_temperature(float _temp, float _threshold);
void display_start_page(void);
void display_refresh(void);
void display_generic_message(void);
#endif /* INC_DISPLAY_DRIVER_H_ */

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
// Define which spi bus to use TFT_VSPI_HOST or TFT_HSPI_HOST
#define SPI_BUS TFT_HSPI_HOST
// ==========================================================
void disp_header(char *info);
void update_header(char *hdr, char *ftr);
void display_driver_init();
void display_driver_demo_test(float _temp);
void display_temperature(float _temp);
#endif /* INC_DISPLAY_DRIVER_H_ */

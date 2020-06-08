/*
 * display_manager.c
 *
 *  Created on: May 22, 2020
 *      Author: Pollock
 *      Company: Digital Forest Sdn Bhd
 */

#include "../inc/display_driver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "tftspi.h"
#include "tft.h"
#include "esp_log.h"
#include "freertos/task.h"
#include "esp_ota_ops.h"
#include "MessageQueue.h"

static const char *TAG = __FILE__;
static char* print_buff[20];
//check this for UI design https://doc.embedded-wizard.de/getting-started-esp-wrover-kit
//ESP Camera Module with QR code scanning: https://www.hackster.io/news/using-a-camera-with-the-esp32-9d6994b34a2b
//---------------------------------
void display_driver_default_header(char *info)
{
	TFT_fillScreen(TFT_BLACK);
	TFT_resetclipwin();

	tft_fg = TFT_WHITE;
	tft_bg = TFT_NAVY;

    if (tft_width < 240) TFT_setFont(DEF_SMALL_FONT, NULL);
	else TFT_setFont(DEFAULT_FONT, NULL);
	TFT_fillRect(0, 0, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);
	TFT_drawRect(0, 0, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);

	TFT_fillRect(0, tft_height-TFT_getfontheight()-9, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);
	TFT_drawRect(0, tft_height-TFT_getfontheight()-9, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);

	TFT_print(DISPLAY_DEVICE_NAME, CENTER, 4);
	TFT_print(info, CENTER, tft_height-TFT_getfontheight()-5);
	tft_bg = TFT_BLACK;
	TFT_setclipwin(0,TFT_getfontheight()+9, tft_width-1, tft_height-TFT_getfontheight()-10);
}

//---------------------------------------------
void display_driver_error_header(char *hdr, char *ftr)
{

	TFT_saveClipWin();
	TFT_resetclipwin();


	tft_fg = TFT_WHITE;
	tft_bg = TFT_RED;
    if (tft_width < 240) TFT_setFont(DEF_SMALL_FONT, NULL);
	else TFT_setFont(DEFAULT_FONT, NULL);

	if (hdr) {
		TFT_fillRect(1, 1, tft_width-3, TFT_getfontheight()+6, TFT_RED);
		TFT_drawRect(1, 1, tft_width-3, TFT_getfontheight()+6, TFT_RED);
		TFT_print(hdr, CENTER, 4);
	}

	if (ftr) {
		TFT_fillRect(1, tft_height-TFT_getfontheight()-8, tft_width-3, TFT_getfontheight()+6, TFT_RED);
		TFT_drawRect(1, tft_height-TFT_getfontheight()-8, tft_width-3, TFT_getfontheight()+6, TFT_RED);
		if (strlen(ftr) == 0) TFT_print(ftr, CENTER, tft_height-TFT_getfontheight()-5);
		else TFT_print(ftr, CENTER, tft_height-TFT_getfontheight()-5);
	}

	tft_bg = TFT_BLACK;

	TFT_restoreClipWin();
}

static void ble_status_callback(void *msg) {
	if (*((bool* )msg) == true)
		display_driver_default_header(print_buff);
	else if (*((bool* )msg) == false)
	{
		display_refresh();
		display_driver_error_header(DISPLAY_BLE_DISCONNECTED_MSG, DISPLAY_ERROR_MSG);
	}
}

static void sensor_update_callback(void *msg) {
	//ESP_LOGI(TAG, "FLOAT Val : %lf\r\n", *((float* )msg));
	display_temperature(*((float* )msg), FEVER_TEMPERATURE_THRESHOLD);
}
void display_driver_init()
{
	    esp_err_t ret;

		tft_max_rdclock = SPI_MAX_CLOCK_FREQ; 	    //Pins MUST be initialized before SPI interface initialization
	    TFT_PinsInit();

	    spi_lobo_device_handle_t spi;

	    spi_lobo_bus_config_t buscfg={
	        .miso_io_num=PIN_NUM_MISO,				// set SPI MISO pin
	        .mosi_io_num=PIN_NUM_MOSI,				// set SPI MOSI pin
	        .sclk_io_num=PIN_NUM_CLK,				// set SPI CLK pin
	        .quadwp_io_num=-1,
	        .quadhd_io_num=-1,
			.max_transfer_sz = SPI_MAX_DATA_TRANSFER_SIZE,
	    };

	    spi_lobo_device_interface_config_t devcfg={
	        .clock_speed_hz=SPI_MAX_CLOCK_FREQ,                // Initial clock out at 8 MHz
	        .mode=0,                                // SPI mode 0
	        .spics_io_num=-1,                       // we will use external CS pin
			.spics_ext_io_num=PIN_NUM_CS,           // external CS pin
			.flags=LB_SPI_DEVICE_HALFDUPLEX,        // ALWAYS SET  to HALF DUPLEX MODE!! for display spi
	    };


		// ==== Initialize the SPI bus and attach the LCD to the SPI bus ====
		ret=spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
	    assert(ret==ESP_OK);
		tft_disp_spi = spi;

		// ==== Test select/deselect ====
		ret = spi_lobo_device_select(spi, 1);
	    assert(ret==ESP_OK);
		ret = spi_lobo_device_deselect(spi);
	    assert(ret==ESP_OK);

		// ==== Initialize the Display ====
		TFT_display_init();

	#ifdef TFT_START_COLORS_INVERTED
		TFT_invertDisplay(1);
	#endif

	    // ==== Set SPI clock used for display operations ====
		spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
		ESP_LOGI("TAG","SPI: Changed speed to %u\r\n", spi_lobo_get_speed(spi));
		TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
		TFT_setRotation(LANDSCAPE_FLIP);
		TFT_resetclipwin();
		display_start_page();
		display_refresh();
		display_driver_error_header(DISPLAY_BLE_DISCONNECTED_MSG, DISPLAY_ERROR_MSG);
		MessageQueue_RegisterMsg(bluetooth, ble_status_callback);
		MessageQueue_RegisterMsg(sensor, sensor_update_callback);

}

void display_start_page()
{
	TFT_setFont(DEJAVU24_FONT, NULL); //DEJAVU18_FONT
		int tempy = TFT_getfontheight() + 4;
		tft_fg = TFT_ORANGE;
		TFT_print(DISPLAY_DEVICE_NAME, CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 - tempy);

		TFT_setFont(UBUNTU16_FONT, NULL);
		tft_fg = TFT_CYAN;
		TFT_print(DISPLAY_POWERED_BY, CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 + tempy-20);
		TFT_print(DISPLAY_DF, CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 + tempy);
		vTaskDelay(2000 / portTICK_RATE_MS);
		esp_app_desc_t *app_info = esp_ota_get_app_description();
		sprintf(print_buff, "Version %s", app_info->version);


}

//=============
void display_temperature(float _temp, float _threshold)
{

	color_t tempFontColour = (_temp>_threshold) ? TFT_RED:TFT_WHITE;

	TFT_setFont(DEJAVU18_FONT, NULL);

	int tempy1 = TFT_getfontheight() - 40;
	tft_fg = tempFontColour;
	TFT_print("                       C", CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 + tempy1);

	char* tmp_buff[20];
	sprintf(tmp_buff, "%2.1f", _temp);

		TFT_setFont(FONT_7SEG, NULL);
		set_7seg_font_atrib(16, 3, 1, tempFontColour);
		int tempy = TFT_getfontheight() - 25;
		tft_fg = tempFontColour;
		TFT_print(tmp_buff, CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 - tempy);


}

void display_generic_message()
{


	TFT_setFont(UBUNTU16_FONT, NULL); //DEJAVU18_FONT

			int tempy = TFT_getfontheight() - 25;
			tft_fg = TFT_WHITE;
			char* tmp_buff =  "Measuring Temperature";
			TFT_print(tmp_buff, CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 - tempy - 20);

}

void display_refresh()
{
	TFT_fillScreen(TFT_BLACK);
	TFT_resetclipwin();
}


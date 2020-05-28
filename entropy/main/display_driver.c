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

static const char *TAG = "display_driver";
//check this for UI design https://doc.embedded-wizard.de/getting-started-esp-wrover-kit
//ESP Camera Module with QR code scanning: https://www.hackster.io/news/using-a-camera-with-the-esp32-9d6994b34a2b
//---------------------------------
void disp_header(char *info)
{
	TFT_fillScreen(TFT_BLACK);
	TFT_resetclipwin();

	tft_fg = TFT_WHITE;
	tft_bg = TFT_NAVY;//(color_t){ 64, 64, 64 };

    if (tft_width < 240) TFT_setFont(DEF_SMALL_FONT, NULL);
	else TFT_setFont(DEFAULT_FONT, NULL);
	TFT_fillRect(0, 0, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);
	TFT_drawRect(0, 0, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);

	TFT_fillRect(0, tft_height-TFT_getfontheight()-9, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);
	TFT_drawRect(0, tft_height-TFT_getfontheight()-9, tft_width-1, TFT_getfontheight()+8, TFT_NAVY);

	TFT_print("BLE CONNECTED", CENTER, 4);
	//_dispTime();
	TFT_print(info, CENTER, tft_height-TFT_getfontheight()-5);
	tft_bg = TFT_BLACK;
	TFT_setclipwin(0,TFT_getfontheight()+9, tft_width-1, tft_height-TFT_getfontheight()-10);
}

//---------------------------------------------
void update_header(char *hdr, char *ftr)
{
	color_t last_fg, last_bg;

	TFT_saveClipWin();
	TFT_resetclipwin();

	Font curr_font = tft_cfont;
	last_bg = tft_bg;
	last_fg = tft_fg;
	tft_fg = TFT_WHITE;
	tft_bg = TFT_RED;
    if (tft_width < 240) TFT_setFont(DEF_SMALL_FONT, NULL);
	else TFT_setFont(DEFAULT_FONT, NULL);

	if (hdr) {
		TFT_fillRect(1, 1, tft_width-3, TFT_getfontheight()+6, tft_bg);
		TFT_print(hdr, CENTER, 4);
	}

	if (ftr) {
		TFT_fillRect(1, tft_height-TFT_getfontheight()-8, tft_width-3, TFT_getfontheight()+6, tft_bg);
		if (strlen(ftr) == 0) TFT_print(ftr, CENTER, tft_height-TFT_getfontheight()-5);
		else TFT_print(ftr, CENTER, tft_height-TFT_getfontheight()-5);
	}

	tft_cfont = curr_font;
	tft_fg = last_fg;
	tft_bg = last_bg;

	TFT_restoreClipWin();
}

void display_driver_init()
{
	    esp_err_t ret;

		tft_max_rdclock = SPI_MAX_CLOCK_FREQ;
	    // === Pins MUST be initialized before SPI interface initialization ===
	    TFT_PinsInit();

	    // ====  CONFIGURE SPI DEVICES(s)  ====================================================================================

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

	    vTaskDelay(500 / portTICK_RATE_MS);

		// ==== Initialize the SPI bus and attach the LCD to the SPI bus ====

		ret=spi_lobo_bus_add_device(SPI_BUS, &buscfg, &devcfg, &spi);
	    assert(ret==ESP_OK);
		ESP_LOGI("TAG","SPI: display device added to spi bus (%d)\r\n", SPI_BUS);
		tft_disp_spi = spi;

		// ==== Test select/deselect ====
		ret = spi_lobo_device_select(spi, 1);
	    assert(ret==ESP_OK);
		ret = spi_lobo_device_deselect(spi);
	    assert(ret==ESP_OK);

	    ESP_LOGI("TAG","SPI: attached display device, speed=%u\r\n", spi_lobo_get_speed(spi));
	    ESP_LOGI("TAG","SPI: bus uses native pins: %s\r\n", spi_lobo_uses_native_pins(spi) ? "true" : "false");
		// ================================
		// ==== Initialize the Display ====

	    ESP_LOGI("TAG","SPI: display init...\r\n");
		TFT_display_init();

	#ifdef TFT_START_COLORS_INVERTED
		TFT_invertDisplay(1);
	#endif


		// ---- Detect maximum read speed ----
		tft_max_rdclock = find_rd_speed();
		ESP_LOGI("TAG","SPI: Max rd speed = %u\r\n", tft_max_rdclock);

	    // ==== Set SPI clock used for display operations ====
		spi_lobo_set_speed(spi, DEFAULT_SPI_CLOCK);
		ESP_LOGI("TAG","SPI: Changed speed to %u\r\n", spi_lobo_get_speed(spi));
		TFT_setGammaCurve(DEFAULT_GAMMA_CURVE);
		TFT_setRotation(LANDSCAPE);
		TFT_resetclipwin();

		TFT_setFont(DEJAVU24_FONT, NULL); //DEJAVU18_FONT
			int tempy = TFT_getfontheight() + 4;
			tft_fg = TFT_ORANGE;
			TFT_print("Sauna32 Lite", CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 - tempy);
			TFT_setFont(UBUNTU16_FONT, NULL);
			tft_fg = TFT_CYAN;
			TFT_print("Powered by", CENTER, LASTY+tempy+2);
			TFT_print("Digital Forest", CENTER, LASTY+tempy);
			vTaskDelay(5000 / portTICK_RATE_MS);
		disp_header("SAUNA32 DEMO");
		vTaskDelay(5000 / portTICK_RATE_MS);
		update_header("BLE DISCONNECTED", "SAUNA32");

}

//=============
void display_temperature(float _temp)
{

	TFT_setFont(DEJAVU18_FONT, NULL);

	int tempy1 = TFT_getfontheight() - 40;
	tft_fg = TFT_WHITE;
	TFT_print("                       C", CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 + tempy1);

	char* tmp_buff[20];
	sprintf(tmp_buff, "%0.1f", _temp);

		TFT_setFont(FONT_7SEG, NULL);
		set_7seg_font_atrib(16, 3, 1, TFT_WHITE);
		int tempy = TFT_getfontheight() - 25;
		tft_fg = TFT_WHITE;
		TFT_print(tmp_buff, CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 - tempy);
//		ESP_LOGI("TAG", "Temp is %d ", tempy);
//		ESP_LOGI("TAG", "Pos is %d ", (tft_dispWin.y2-tft_dispWin.y1)/2 );
}

void display_driver_demo_test(float _temp)
{
	disp_header("ESP32 TFT DEMO");
	TFT_setFont(TOONEY32_FONT, NULL);

	int tempy = TFT_getfontheight() + 4;
	tft_fg = TFT_ORANGE;
	TFT_print("Sauna32", CENTER, (tft_dispWin.y2-tft_dispWin.y1)/2 - tempy);

	//TFT_resetclipwin();
	vTaskDelay(5000 / portTICK_RATE_MS);
	disp_header("7-SEG FONT DEMO");


		char* tmp_buff[20];
		tft_fg = TFT_ORANGE;
		sprintf(tmp_buff, "%0.2f °C", _temp);
		TFT_setFont(FONT_7SEG, NULL);

        if ((tft_width < 240) || (tft_height < 240)) {
        	set_7seg_font_atrib(12, 2, 1, TFT_DARKGREY);
        	ESP_LOGI("TEST", "Executing 1");

	}
		else {
			set_7seg_font_atrib(12, 2, 1, TFT_DARKGREY);
			ESP_LOGI("TEST", "Executing 2");
		}
		//TFT_clearStringRect(12, y, tmp_buff);
        int offset = 25;
        int y = ((tft_dispWin.y2-tft_dispWin.y1)-TFT_getfontheight()-2) - offset;
        ESP_LOGI("TEST","the offset value for the font is %d", (tft_dispWin.y2-tft_dispWin.y1)-TFT_getfontheight()-2); //63 //49 for font size 12
        ESP_LOGI("TEST","%d\r\n",y);
        TFT_print(tmp_buff, CENTER, y);
}


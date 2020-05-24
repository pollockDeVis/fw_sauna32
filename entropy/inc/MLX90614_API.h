#ifndef _MLX614_API_H_
#define _MLX614_API_H_
#include "esp_system.h"
#include <stdint.h>

// RAM
#define MLX90614_RAWIR1 0x04
#define MLX90614_RAWIR2 0x05
#define MLX90614_TA 0x06
#define MLX90614_TOBJ1 0x07
#define MLX90614_TOBJ2 0x08
// EEPROM
#define MLX90614_TOMAX 0x20
#define MLX90614_TOMIN 0x21
#define MLX90614_PWMCTRL 0x22
#define MLX90614_TARANGE 0x23
#define MLX90614_EMISS 0x24
#define MLX90614_CONFIG 0x25
#define MLX90614_ADDR 0x2E
#define MLX90614_ID1 0x3C
#define MLX90614_ID2 0x3D
#define MLX90614_ID3 0x3E
#define MLX90614_ID4 0x3F
#define MLX90614_MAX_TEMP 0x7FFF

int MLX90614_DumpEE(uint8_t slaveAddr, uint16_t *eeData);
int MLX90614_GetTa(uint8_t slaveAddr, float *ta);
int MLX90614_GetTo(uint8_t slaveAddr, float *to);
int MLX90614_GetTo2(uint8_t slaveAddr, float *to2);
int MLX90614_GetEmissivity(uint8_t slaveAddr, float *emissivity);
int MLX90614_SetEmissivity(uint8_t slaveAddr, float value);

    
#endif


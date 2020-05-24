#include "MLX90614_SMBus_Driver.h"
#include "esp_system.h"
#include <math.h>
#include "MLX90614_API.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "MLX90614_API";
//------------------------------------------------------------------------------
  
int MLX90614_DumpEE(uint8_t slaveAddr, uint16_t *eeData)
{
     int error = 0;
     char address = MLX90614_TOMAX;
     uint16_t *p = eeData;
     
     while (address < 0x40 && error == 0)
     {
        error = MLX90614_SMBusRead(slaveAddr, address, p);
        address = address + 1;
        p = p + 1;
     }   
         
     return error;
}

//------------------------------------------------------------------------------

int MLX90614_GetTa(uint8_t slaveAddr, float *ta)
{
    int error = 0;
    uint16_t data = 0;
    
    error = MLX90614_SMBusRead(slaveAddr, MLX90614_TA, &data);
    
    if (data > MLX90614_MAX_TEMP)
    {
        return -4;
    }
        
    *ta = (float)data * 0.02f - 273.15;
        
    return error;
}    

//------------------------------------------------------------------------------

int MLX90614_GetTo(uint8_t slaveAddr, float *to)
{
    int error = 0;
    uint16_t data = 0;
    
    error = MLX90614_SMBusRead(slaveAddr, MLX90614_TOBJ1, &data);
    
    if (data > MLX90614_MAX_TEMP)
    {
        return -4;
    }
    
    if (error == 0)
    {
        *to = (float)data * 0.02f - 273.15;
    } 
    
    return error;
} 

//------------------------------------------------------------------------------

int MLX90614_GetTo2(uint8_t slaveAddr, float *to2)
{
    int error = 0;
    uint16_t data = 0;
    
    error = MLX90614_SMBusRead(slaveAddr, MLX90614_TOBJ2, &data);
    
    if (data > MLX90614_MAX_TEMP)
    {
        return -4;
    }
    
    if (error == 0)
    {
        *to2 = (float)data * 0.02 - 273.15;
    }        
    
    return error;
}   


//------------------------------------------------------------------------------

int MLX90614_GetEmissivity(uint8_t slaveAddr, float *emissivity)
{
    int error = 0;
    uint16_t data = 0;
    error = MLX90614_SMBusRead(slaveAddr, MLX90614_EMISS, &data);
    ESP_LOGI(TAG, "Read Data %x", data );
    *emissivity = (float)data / 0xFFFF;
    return error;
}

//------------------------------------------------------------------------------

int MLX90614_SetEmissivity(uint8_t slaveAddr, float value)
{
	int error = 0;
	    uint16_t data;
	    uint16_t curE;
	    uint16_t newE = 0;
	    float temp = 0;

	    if(value > 1.0 || value < 0.05)
	    {
	        return -6;
	    }

	    temp = value * 65535;
	    newE = temp;

	    error = MLX90614_SMBusRead(slaveAddr, MLX90614_EMISS, &curE);

	    if(error == 0)
	    {

	            if(error == 0)
	            {
	                error = MLX90614_SMBusWrite(slaveAddr, MLX90614_EMISS, 0x00); //0x0000 ruined calibration should be 0x00
	                vTaskDelay(10 / portTICK_RATE_MS);
	                if(error == 0)
	                {
	                    error = MLX90614_SMBusWrite(slaveAddr, MLX90614_EMISS, newE);
	                    vTaskDelay(10 / portTICK_RATE_MS);

	                }
	            }

	    }

	    return error;
}
 

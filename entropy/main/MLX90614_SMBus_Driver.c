#include "driver/i2c.h"
#include "esp_log.h"
#include "MLX90614_SMBus_Driver.h"

#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */


static const char *TAG = "MLX90614_Module";


void MLX90614_SMBusInit(uint8_t sda_gpio, uint8_t scl_gpio, int freq){   
    i2c_config_t MLX90614_config = {
        .mode = I2C_MODE_MASTER,       /*!< I2C mode */
        .sda_io_num = sda_gpio,        /*!< GPIO number for I2C sda signal */
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  /*!< Internal GPIO pull mode for I2C sda signal*/
        .scl_io_num = scl_gpio,       /*!< GPIO number for I2C scl signal */
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  /*!< Internal GPIO pull mode for I2C scl signal*/
        .master.clk_speed = freq    /*!< I2C clock frequency for master mode, (no higher than 1MHz for now) */
    };
    i2c_param_config(I2C_NUM_0, &MLX90614_config);
    i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

int MLX90614_SMBusRead(uint8_t slaveAddr, uint8_t reg_addr, uint16_t *data){
    
    uint8_t chip_addr; 
    uint8_t data_addr;                          
    uint8_t pec;                               
    uint16_t *p;

    p = data;
    chip_addr = (slaveAddr << 1);
    data_addr = reg_addr;
    pec = chip_addr;
    uint8_t recived_data[3] = {0,0,0};

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr | WRITE_BIT, ACK_CHECK_EN); //start-slave address-write-ack
    i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN); //command-ack
    i2c_master_start(cmd); //repeat start seq
    chip_addr = chip_addr | 0x01; //This is important for the read. Not sure what it does
    i2c_master_write_byte(cmd, chip_addr | READ_BIT, ACK_CHECK_EN); //slave add-read-ack
    i2c_master_read(cmd, recived_data, 2, ACK_VAL); //received data-ackx2
    i2c_master_read_byte(cmd, recived_data + 2, NACK_VAL);  //received_data+2 shifting pointer by 2
    i2c_master_stop(cmd);  
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);

    if (ret == ESP_OK) {
        pec = Calculate_PEC(0, pec);
        pec = Calculate_PEC(pec, data_addr);
        pec = Calculate_PEC(pec, chip_addr);
        pec = Calculate_PEC(pec, recived_data[0]);
        pec = Calculate_PEC(pec, recived_data[1]);

    } else if (ret == ESP_ERR_TIMEOUT) {
        ESP_LOGW(TAG, "Bus is busy");
    } else {
       // ESP_LOGW(TAG, "Read failed");
    }
    if (pec != recived_data[2]) //comparing last packet with calculated PEC
    {
        return -2;
    }
        
    *p = (uint16_t)recived_data[1]*256 + (uint16_t)recived_data[0];
    return 0;   
}

int MLX90614_SMBusWrite(uint8_t slaveAddr, uint8_t writeAddress, uint16_t data)
{
	//ESP_LOGI(TAG, "Data incoming %d", data);
		uint8_t chip_addr;
		chip_addr = (slaveAddr << 1);
	    uint8_t send_data[2] = {0,0};


	    uint8_t pec;
		uint16_t incomingData;
		incomingData = data;

	    send_data[0] = data & 0xFF; //change to FF and check
	    send_data[1] = data >> 8;

	    pec = Calculate_PEC(0, chip_addr);
	    pec = Calculate_PEC(pec, writeAddress);
	    pec = Calculate_PEC(pec, send_data[0]);
	    pec = Calculate_PEC(pec, send_data[1]);


	    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
	    i2c_master_start(cmd); //start
	    i2c_master_write_byte(cmd, chip_addr | WRITE_BIT, ACK_CHECK_EN); //slave add-write-ack
	    i2c_master_write_byte(cmd, writeAddress, ACK_CHECK_EN); //command-ack
	    i2c_master_write(cmd, send_data, 2, ACK_CHECK_EN);
	    i2c_master_write_byte(cmd, pec, NACK_VAL);
	    i2c_master_stop(cmd);
	    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_RATE_MS);
	    i2c_cmd_link_delete(cmd);


	    if (ret == ESP_OK) {
	        ESP_LOGI(TAG, "Write OK");
	    } else if (ret == ESP_ERR_TIMEOUT) {
	        ESP_LOGW(TAG, "Bus is busy");
	    } else {
	        ESP_LOGW(TAG, "Write Failed");
	    }

	    uint16_t dataCheck = 0;
	    MLX90614_SMBusRead(slaveAddr, writeAddress, &dataCheck);

	    return 0;
}



uint8_t Calculate_PEC (uint8_t initPEC, uint8_t newData)
{
    uint8_t data;
    uint8_t bitCheck;

    data = initPEC ^ newData;
    
    for (int i=0; i<8; i++ )
    {
        bitCheck = data & 0x80;
        data = data << 1;
        
        if (bitCheck != 0)
        {
            data = data ^ 0x07;
        }
        
    }
    return data;
}


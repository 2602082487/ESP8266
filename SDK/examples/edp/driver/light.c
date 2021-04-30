#include "esp_common.h"
#include "c_types.h"
#include "driver/i2c_master.h"
#include "driver/delay.h"

//void write_cmd(et_uint8 I2C_Command)//Ğ´ÃüÁî
//{	
//	i2c_master_start();
//	i2c_master_writeByte(0x78);	//Slave address,SA0=0
//	i2c_master_waitAck();
//	i2c_master_writeByte(0x00);	//write command
//	i2c_master_waitAck();
//	i2c_master_writeByte(I2C_Command);
//	i2c_master_waitAck();
//	i2c_master_stop();
//}

void write_data(uint8 I2C_Data)//Ğ´Êı¾İ
{
	i2c_master_start();
	i2c_master_writeByte(0x46);	//D/C#=0; R/W#=0
	i2c_master_waitAck();
	i2c_master_writeByte(I2C_Data);	//write data
	i2c_master_waitAck();
	i2c_master_stop();
}



uint16 read_data()
{
	uint16 data;
	i2c_master_start();
	i2c_master_writeByte(0x47);
	i2c_master_waitAck();
	data=i2c_master_readByte();
	i2c_master_send_ack();
	data=data<<8;
	data|=i2c_master_readByte();
	i2c_master_waitAck();
	i2c_master_stop();
	return data;
}


void bh1750_PowerDown()
{
	write_data(0x00);
}


void bh1750_PowerOn()
{
	write_data(0x01);
}

void bh1750_Reset()
{
	write_data(0x07);
}

void bh1750_SetContinuHMode()
{
	write_data(0x10);
}

void bh1750_init()
{
	i2c_master_gpio_init();
	bh1750_PowerOn();
	bh1750_SetContinuHMode();
	delay_ms(180);
}

uint16 bh1750_GetLightValue()
{
	uint16 data;
	data=read_data();
	delay_ms(180);
	return data;
	
}



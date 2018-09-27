#include "si7021.h"
#include "config.h"

#undef 	DEVICE
#define DEVICE 					"[SI7021]: "

/* global variable. */
si7021_t  			si7021;

/* I2C Command Table ----------------------------------------------------------*/
#define MRH_HLD_MSTR_MODE				0xE5  			/* Measure Relative Humidity, Hold Master Mode */
#define MRH_NO_HLD_MSTR_MODE			0xF5 			/* Measure Relative Humidity, No Hold Master Mode */
#define MT_HLD_MSTR_MODE				0xE3			/* Measure Temperature, Hold Master Mode */
#define MT_NO_HLD_MSTR_MODE				0xF3			/* Measure Temperature, No Hold Master Mode */
#define RD_T_FROM_PRHM					0xE0			/* Read Temperature Value from Previous RH Measurememt */
#define SI7021_RESET					0xFE			/* Reset */
#define WR_RHT_REGISTER1				0xE6			/* Write RH/T User Register 1 */
#define RD_RHT_REGISTER1				0xE7			/* Read RH/T User Register 1 */
#define ELECTRONIC_ID_BYTE1_CMD1		0xFA			/* Read Electronic ID 1st Byte */
#define ELECTRONIC_ID_BYTE1_CMD2		0x0F			/* Read Electronic ID 1st Byte */		
#define ELECTRONIC_ID_BYTE2_CMD1		0xFC			/* Read Electronic ID 2nd Byte */
#define ELECTRONIC_ID_BYTE2_CMD2		0xC9			/* Read Electronic ID 2nd Byte */
#define FIRMWARE_REVERSION_CMD1			0x84			/* Read Firmware Revision */
#define FIRMWARE_REVERSION_CMD2			0xB8			/* Read Firmware Revision */
#define SI7021_ADDR 					0x40



/* 仅供内部使用的代码 */
static int32_t si7021_write_cmd(si7021_t *dev, uint8_t cmd);
static int32_t si7021_read_reg(si7021_t *dev, uint8_t reg, uint8_t *val);
static int32_t si7021_get_humidity(si7021_t *dev, uint16_t *val);
static int32_t si7021_get_temperature(si7021_t *dev, uint16_t *val);


/*
 * SI7021 初始化
*/
static int32_t si7021_init(si7021_t *dev)
{
	uint8_t tmp;
	assert(dev);
	si7021_write_cmd(dev, SI7021_RESET);
	delay_10ms(1);
	si7021_read_reg(dev, RD_RHT_REGISTER1, &tmp);
	if (tmp & (1 << 6)) {
		err("VDD is low!\r\n");
		return -1;
	}
	debug(DEVICE "SI7021 初始化成功!\r\n");
	return 0;
}


static int32_t si7021_check_device(si7021_t *dev)
{
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	i2c->start(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK) {
		i2c->stop(i2c);
		err("SI7021 not found!\r\n");
		return -1;
	}
	i2c->stop(i2c);
	debug(DEVICE "SI7021 found!\r\n");
	return 0;
}

/**
  ******************************************************************************
  * @brief   向SI7021写一个字节CMD
  * @param   cmd : SI7021内部CMD
  * @retval  0 写成功； 1 写失败
  ******************************************************************************
  */
static int32_t si7021_write_cmd(si7021_t *dev, uint8_t cmd)
{
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	mutex_lock(dev->mutex);
	i2c->start(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	delay_10ms(1);
	i2c->send_byte(i2c, cmd);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->stop(i2c);
	mutex_unlock(dev->mutex);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("I2C no ack!");
	mutex_unlock(dev->mutex);
	return -1;
}

static int32_t si7021_read_reg(si7021_t *dev, uint8_t reg, uint8_t *val)
{
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	assert(val);
	mutex_lock(dev->mutex);
	i2c->start(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	delay_10ms(1);
	i2c->send_byte(i2c, reg);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	delay_10ms(1);
	i2c->restart(i2c);							/* generate Sr condition. */
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	delay_10ms(1);
	i2c->recv_byte(i2c, val);
	i2c->send_nack(i2c);
	i2c->stop(i2c);
	mutex_unlock(dev->mutex);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("I2C no ack!\r\n");
	mutex_unlock(dev->mutex);
	return -1;
}

/**
  ******************************************************************************
  * @brief   向SI7021发送测量温度命令, 注意延迟11ms
  * @param   None
  * @retval  0 成功； 1 失败
  ******************************************************************************
  */
static int32_t si7021_get_humidity(si7021_t *dev, uint16_t *val)
{
	uint8_t tmp[2];
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	i2c->start(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_WR);
	delay_10ms(1);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, MRH_HLD_MSTR_MODE);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	delay_10ms(2);
	i2c->restart(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	delay_10ms(2);
	i2c->recv_byte(i2c, &tmp[0]);
	i2c->send_ack(i2c);	
	i2c->recv_byte(i2c, &tmp[1]);
	i2c->send_nack(i2c);
	i2c->stop(i2c);
	
	*val = (uint16_t)tmp[0] << 8 | tmp[1];
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("I2C no ack!\r\n");
	return -1;	
}

/**
  ******************************************************************************
  * @brief   向SI7021发送测量温度命令, 注意延迟11ms
  * @param   None
  * @retval  0 成功； 1 失败
  ******************************************************************************
  */
static int32_t si7021_get_temperature(si7021_t *dev, uint16_t *val)
{
	uint8_t tmp[2];
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	assert(val)
	i2c->start(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, MT_HLD_MSTR_MODE);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	delay_10ms(2);
	i2c->restart(i2c);
	i2c->send_byte(i2c, SI7021_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	delay_10ms(2);
	i2c->recv_byte(i2c, &tmp[0]);
	i2c->send_ack(i2c);	
	i2c->recv_byte(i2c, &tmp[1]);
	i2c->send_nack(i2c);
	i2c->stop(i2c);
	
	*val = (uint16_t)tmp[0] << 8 | tmp[1];
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("I2C no ack!\r\n");
	return -1;	
}

static int32_t si7021_read_data(si7021_t *dev)
{
	uint16_t humidityval, temperatureval;
	
	assert(dev);
	if (si7021_get_humidity(dev, &humidityval) || si7021_get_temperature(dev, &temperatureval)) {
		err("read si7021 failed!\r\n");
		return -1;
	}
	
	dev->humidity = ( ((uint32_t)humidityval * 12500) >> 16 ) - 600;
	dev->temperature = ( ((uint32_t)temperatureval * 17572) >> 16 ) - 4685;
	return 0;
	
}

int32_t si7021_register(si7021_t *dev, i2c_bus_t *i2c)
{
	assert(dev);
	assert(i2c);
	dev->i2c = i2c;
	dev->check_device = si7021_check_device;
	dev->init = si7021_init;
	dev->read_data = si7021_read_data;
	dev->mutex = mutex_create("si7021");
	return 0;
}








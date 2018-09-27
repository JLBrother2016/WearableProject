#include "bh1750.h"
#include "config.h"

#undef 	DEVICE
#define DEVICE 					"[BH1750]: "

/* global variable. */
bh1750_t bh1750;

/* BH1750操作码定义 */
#define  POWER_DOWN 					0x00		/* No active state. 进入掉电模式，芯片上电后缺省就是PowerDown模式 */
#define  POWER_ON 						0x01		/* Waiting for measurement command. 上电，等待测量命令 */
#define  BH1750_RESET					0x07		/* Reset data register value. Reset command is not acceptable in power down mode.清零数据寄存器（Power Down 模式无效） */
#define  CNTNS_H_MODE					0x10		/* Start measurement at 1lx resolution. Measurement time is typically 120ms. 连续高分辨率测量模式（测量时间120ms,最大180ms）*/
#define  CNTNS_H_MODE2					0x11		/* Start measurement at 0.5lx resolution. Measurement time is typically 120ms. 连续高分辨率测量模式（测量时间120ms）*/
#define  CNTNS_L_MODE					0x13		/* Start measurement at 4lx resolution. Measurement time is typically 16ms. 连续低分辨率测量模式（测量时间16ms）*/
#define  ONE_TIME_H_MODE				0x20		/* Start measurement at 1lx resolution. Measurement time is typically 120ms. 单次高分辨率测量模式，之后自动进入Power Down */
													/* It is automatically set to Power Down mode after measurement. */
#define  ONE_TIME_H_MODE2				0x21		/* Start measurement at 0.5lx resolution. Measurement time is typically 120ms. 单次高分辨率测量模式2,之后自动进入Power Down */
													/* It is automatically set to Power Down mode after measurement. */
#define  ONE_TIME_L_MODE 				0x23		/* Start measurement at 4lx resolution. Measurement time is typically 120ms. 单次低分辨率测量模式，自后自动进入Power Down */
													/* It is automatically set to Power Down mode after measurement. */
													
#define BH1750FVI_ADDR 	0x23   						/* ADDR引脚接地 */


int32_t bh1750_init(bh1750_t *dev)
{
	assert(dev);	
	dev->write_cmd(dev, POWER_ON);										/* 芯片上电 */
	dev->ioctl(dev, BH1750_CMD_SET_MEASURE_MODE, CONT_H_MODE2);			/* 默认高分辨率连续测量模式2 */							
	dev->ioctl(dev, BH1750_CMD_SET_SENSITIVITY, 69);					/* 芯片缺省灵敏度倍率为69 */																		
	
	debug(DEVICE "BH1750 初始化成功!\r\n");
	return 0;
}


/**
  ******************************************************************************
  * @brief   读取BH1750测量结果.主程序需要在 bh1750_init() 执行之后 180ms 才能
  * 		 读到正确数据。连续测量模式下，之后主程序可以定时调用本函数读取光强度
  * 		 数据，间隔时间需要大于180ms
  * @param   None
  * @retval  原始的测量数据 ,结果未转换
  ******************************************************************************
  */
int32_t bh1750_read_data(bh1750_t *dev)
{	
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	i2c->start(i2c);
	i2c->send_byte(i2c, BH1750FVI_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	
	i2c->recv_byte(i2c, &dev->buffer[0]);
	i2c->send_ack(i2c);
	i2c->recv_byte(i2c, &dev->buffer[1]);
	i2c->send_nack(i2c);
	i2c->stop(i2c);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;	
}


/**
  ******************************************************************************
  * @brief   BH1750写命令
  * @param   _chCMD : BH1750内部命令字
  * @retval  None
  ******************************************************************************
  */
int32_t bh1750_write_cmd(bh1750_t *dev, uint8_t cmd)
{
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	i2c->start(i2c);										/* I2C 总线开始信号 */
	i2c->send_byte(i2c, BH1750FVI_ADDR << 1 | I2C_WR);		/* 发送设备地址+写信号 */
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, cmd);								/* 发送1字节命令 */
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->stop(i2c);											/* I2C1 总线停止信号 */
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;	
}


/**
  ******************************************************************************
  * @brief   读取亮度值
  * 			H-reslution mode Illuminance per 1 count (lx / count) = 1 / 1.2 * (69 / X)
  *  			H-reslution mode2 Illuminance per 1 count (lx / count) = 1 / 1.2 * (69 / X) / 2
  * 			1.2 Measurement accuracy
  * 			69 Default value of MTreg(dec)
  * 			X MTreg value
  * @param  None : 
  * @retval 光强度 [Lx]  浮点数 
  ******************************************************************************
  */
int32_t bh1750_get_lx(bh1750_t *dev)
{	
	uint16_t raw_data;
	
	assert(dev);
	dev->read_data(dev);
	raw_data = (uint16_t)dev->buffer[0] << 8 | dev->buffer[1];
	dev->lx = (float)(raw_data * 5 * 69) / (6 * dev->mtreg);
	
	if(dev->mode == CONT_H_MODE2)
		dev->lx = dev->lx / 2;
	
	return 0;
}

int32_t bh1750_check_device(bh1750_t *dev)
{
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	i2c->start(i2c);
	i2c->send_byte(i2c, BH1750FVI_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	
	i2c->stop(i2c);
	debug(DEVICE "BH1750 found!\r\n");
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("No bh1750 device found!\r\n");
	return -1;	
	
}


static int32_t bh1750_ioctl(bh1750_t *dev, uint8_t cmd, int32_t val)
{
	assert(dev);
	switch (cmd) {
	case BH1750_CMD_SET_MEASURE_MODE:
		if (val == CONT_H_MODE) {
			dev->write_cmd(dev, CNTNS_H_MODE);
			dev->mode = CONT_H_MODE;
		} else if (val == CONT_H_MODE2) {
			dev->write_cmd(dev, CNTNS_H_MODE2);
			dev->mode = CONT_H_MODE2;
		} else if (val == CONT_L_MODE) {
			dev->write_cmd(dev, CNTNS_L_MODE);
			dev->mode = CONT_L_MODE;
		} else {
			err("cmd: CMD_SET_MEASURE_MODE has no val: %d\r\n", val);
			return -1;
		}
		break;
		
	case BH1750_CMD_SET_SENSITIVITY:
		if (val <= 31) 
			dev->mtreg = 31;
		else if (val >= 254) 
			dev->mtreg = 254;
		dev->mtreg = val;
		dev->write_cmd(dev, 0x40 + (dev->mtreg >> 5));			/* 改变高3bit */
		dev->write_cmd(dev, 0x60 + (dev->mtreg & 0x1f));		/* 改变低5位 */
		dev->ioctl(dev, BH1750_CMD_SET_MEASURE_MODE, dev->mode);		/* 更改量程范围后，需要重新发送命令设置测量模式　*/	
		break;
	
	default:
		err("bh1750 no such command!\r\n");
		return -1;
	}
	
	return 0;
}

int32_t bh1750_register(bh1750_t *dev, i2c_bus_t *i2c)
{
	assert(dev);
	assert(i2c);	
	dev->i2c = i2c;
	dev->check_device = bh1750_check_device;
	dev->init = bh1750_init;
	dev->read_data = bh1750_read_data;
	dev->write_cmd = bh1750_write_cmd;
	dev->get_lx = bh1750_get_lx;
	dev->ioctl = bh1750_ioctl;
	
	return 0;
}


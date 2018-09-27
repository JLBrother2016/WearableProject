#include "bh1750.h"
#include "config.h"

#undef 	DEVICE
#define DEVICE 					"[BH1750]: "

/* global variable. */
bh1750_t bh1750;

/* BH1750�����붨�� */
#define  POWER_DOWN 					0x00		/* No active state. �������ģʽ��оƬ�ϵ��ȱʡ����PowerDownģʽ */
#define  POWER_ON 						0x01		/* Waiting for measurement command. �ϵ磬�ȴ��������� */
#define  BH1750_RESET					0x07		/* Reset data register value. Reset command is not acceptable in power down mode.�������ݼĴ�����Power Down ģʽ��Ч�� */
#define  CNTNS_H_MODE					0x10		/* Start measurement at 1lx resolution. Measurement time is typically 120ms. �����߷ֱ��ʲ���ģʽ������ʱ��120ms,���180ms��*/
#define  CNTNS_H_MODE2					0x11		/* Start measurement at 0.5lx resolution. Measurement time is typically 120ms. �����߷ֱ��ʲ���ģʽ������ʱ��120ms��*/
#define  CNTNS_L_MODE					0x13		/* Start measurement at 4lx resolution. Measurement time is typically 16ms. �����ͷֱ��ʲ���ģʽ������ʱ��16ms��*/
#define  ONE_TIME_H_MODE				0x20		/* Start measurement at 1lx resolution. Measurement time is typically 120ms. ���θ߷ֱ��ʲ���ģʽ��֮���Զ�����Power Down */
													/* It is automatically set to Power Down mode after measurement. */
#define  ONE_TIME_H_MODE2				0x21		/* Start measurement at 0.5lx resolution. Measurement time is typically 120ms. ���θ߷ֱ��ʲ���ģʽ2,֮���Զ�����Power Down */
													/* It is automatically set to Power Down mode after measurement. */
#define  ONE_TIME_L_MODE 				0x23		/* Start measurement at 4lx resolution. Measurement time is typically 120ms. ���εͷֱ��ʲ���ģʽ���Ժ��Զ�����Power Down */
													/* It is automatically set to Power Down mode after measurement. */
													
#define BH1750FVI_ADDR 	0x23   						/* ADDR���Žӵ� */


int32_t bh1750_init(bh1750_t *dev)
{
	assert(dev);	
	dev->write_cmd(dev, POWER_ON);										/* оƬ�ϵ� */
	dev->ioctl(dev, BH1750_CMD_SET_MEASURE_MODE, CONT_H_MODE2);			/* Ĭ�ϸ߷ֱ�����������ģʽ2 */							
	dev->ioctl(dev, BH1750_CMD_SET_SENSITIVITY, 69);					/* оƬȱʡ�����ȱ���Ϊ69 */																		
	
	debug(DEVICE "BH1750 ��ʼ���ɹ�!\r\n");
	return 0;
}


/**
  ******************************************************************************
  * @brief   ��ȡBH1750�������.��������Ҫ�� bh1750_init() ִ��֮�� 180ms ����
  * 		 ������ȷ���ݡ���������ģʽ�£�֮����������Զ�ʱ���ñ�������ȡ��ǿ��
  * 		 ���ݣ����ʱ����Ҫ����180ms
  * @param   None
  * @retval  ԭʼ�Ĳ������� ,���δת��
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
  * @brief   BH1750д����
  * @param   _chCMD : BH1750�ڲ�������
  * @retval  None
  ******************************************************************************
  */
int32_t bh1750_write_cmd(bh1750_t *dev, uint8_t cmd)
{
	i2c_bus_t *i2c = dev->i2c;
	
	assert(dev);
	i2c->start(i2c);										/* I2C ���߿�ʼ�ź� */
	i2c->send_byte(i2c, BH1750FVI_ADDR << 1 | I2C_WR);		/* �����豸��ַ+д�ź� */
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, cmd);								/* ����1�ֽ����� */
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->stop(i2c);											/* I2C1 ����ֹͣ�ź� */
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;	
}


/**
  ******************************************************************************
  * @brief   ��ȡ����ֵ
  * 			H-reslution mode Illuminance per 1 count (lx / count) = 1 / 1.2 * (69 / X)
  *  			H-reslution mode2 Illuminance per 1 count (lx / count) = 1 / 1.2 * (69 / X) / 2
  * 			1.2 Measurement accuracy
  * 			69 Default value of MTreg(dec)
  * 			X MTreg value
  * @param  None : 
  * @retval ��ǿ�� [Lx]  ������ 
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
		dev->write_cmd(dev, 0x40 + (dev->mtreg >> 5));			/* �ı��3bit */
		dev->write_cmd(dev, 0x60 + (dev->mtreg & 0x1f));		/* �ı��5λ */
		dev->ioctl(dev, BH1750_CMD_SET_MEASURE_MODE, dev->mode);		/* �������̷�Χ����Ҫ���·����������ò���ģʽ��*/	
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


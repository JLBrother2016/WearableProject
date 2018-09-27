#include "bmp180.h"
#include "config.h"

#define BMP180_ADDR 	0x77

#undef 	DEVICE
#define DEVICE 					"[BMP180]: "

/* global variable. */
bmp180_t bmp180;

/*
 * 读取BMP180内部寄存器的值, 2个字节
*/
static int32_t bmp180_read_reg(bmp180_t *bmp, uint8_t reg, void *val)
{
	uint8_t msb, lsb;
	i2c_bus_t *i2c = bmp->i2c;
	
	assert(bmp);
	assert(val);
	i2c->start(i2c);
	i2c->send_byte(i2c, BMP180_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, reg);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->restart(i2c);
	i2c->send_byte(i2c, BMP180_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->recv_byte(i2c, &msb);
	i2c->send_ack(i2c);
	i2c->recv_byte(i2c, &lsb);
	i2c->stop(i2c);
	
	*(uint16_t *)val = (uint16_t)msb << 8 | lsb;
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;
}


/*
 * 写一个字节到BMP180寄存器中
*/
static int32_t bmp180_write_reg(bmp180_t *bmp, uint8_t reg, uint8_t val)
{
	i2c_bus_t *i2c = bmp->i2c;
	
	assert(bmp);
	i2c->start(i2c);
	i2c->send_byte(i2c, BMP180_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, reg);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, val);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->stop(i2c);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;
}


static int32_t bmp180_get_cal_param(bmp180_t *bmp)
{
	assert(bmp);
	bmp->read_reg(bmp, 0xaa, &bmp->ac1);
	bmp->read_reg(bmp, 0xac, &bmp->ac2);
	bmp->read_reg(bmp, 0xae, &bmp->ac3);
	bmp->read_reg(bmp, 0xb0, &bmp->ac4);
	bmp->read_reg(bmp, 0xb2, &bmp->ac5);
	bmp->read_reg(bmp, 0xb4, &bmp->ac6);
	bmp->read_reg(bmp, 0xb6, &bmp->b1);
	bmp->read_reg(bmp, 0xb8, &bmp->b2);
	bmp->read_reg(bmp, 0xba, &bmp->mb);
	bmp->read_reg(bmp, 0xbc, &bmp->mc);
	bmp->read_reg(bmp, 0xbe, &bmp->md);
	debug(DEVICE "BMP1850 初始化成功!\r\n");
	return 0;
}

static int32_t bmp180_get_ut(bmp180_t *bmp)
{
	assert(bmp);
	bmp->write_reg(bmp, 0xf4, 0x2e);
	delay_10ms(1);
	
	if (bmp->read_reg(bmp, 0xf6, &bmp->ut) != 0) {
		err("bmp get ut faile!\r\n");
		return -1;
	}
	return 0;
}

static int32_t bmp180_get_up(bmp180_t *bmp)
{
	assert(bmp);
	bmp->write_reg(bmp, 0xf4, 0x34);
	delay_10ms(1);
	
	if (bmp->read_reg(bmp, 0xf6, &bmp->up) != 0) {
		err("bmp get up failed!\r\n");
		return -1;
	}
	return 0;
}

static int32_t bmp180_read_data(bmp180_t *bmp)
{
	int32_t x1, x2, x3, b3, b5, b6, p;
	uint32_t b4, b7;
	
	assert(bmp);
	if (bmp->get_ut(bmp) != 0) {
		err("bmp get ut err!\r\n");
		return -1;
	}
	if (bmp->get_up(bmp) != 0) {
		err("bmp get tp err!\r\n");
		return -1;
	}
	
	x1 = ( (bmp->ut - (int32_t)bmp->ac6) * (int32_t)bmp->ac5 ) >> 15;
	x2 = ( (int32_t)bmp->mc << 11 ) / ( x1 + bmp->md );
	b5 = x1 + x2;
	bmp->temperature = (b5 + 8) >> 4;

	b6 = b5 - 4000;
	x1 = ( (int32_t)bmp->b2 * ((b6 * b6) >> 12) ) >> 11;
	x2 = (int32_t)bmp->ac2 * b6 >> 11;
	x3 = x1 + x2;

	b3 = (((int32_t)(bmp->ac1) * 4 + x3) + 2) >> 2;
	x1 = (int32_t)bmp->ac3 * b6 >> 13;
	x2 = ((int32_t)bmp->b1 * (b6 * b6) >> 12) >> 16;
	x3 = ((x1 + x2) + 2) >> 2;
	b4 = (int32_t)bmp->ac4 * (uint32_t)(x3 + 32768) >> 15;
	b7 = (uint32_t)(bmp->up - b3) * (50000);

	if (b7 < 0x80000000)
		p = (b7 << 1) / b4;
	else
		p = (b7 / b4) << 1;

	x1 = (p >> 8) * (p >> 8);
	x1 = (x1 * 3038) >> 16;
	x2 = (-7357 * p) >> 16;

	bmp->pressure = p + ((x1 + x2 + 3791) >> 4);
	return 0;
}

static int32_t bmp180_check_device(bmp180_t *bmp)
{
	i2c_bus_t *i2c = bmp->i2c;
	
	i2c->start(i2c);
	i2c->send_byte(i2c, BMP180_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	
	i2c->stop(i2c);
	debug(DEVICE "BMP180 found!\r\n");
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("No BMP180 device found!\r\n");
	return -1;
}

int32_t bmp180_register(bmp180_t *bmp, i2c_bus_t *i2c)
{
	assert(bmp);
	bmp->i2c 			= i2c;
	bmp->check_device   = bmp180_check_device;
	bmp->init 			= bmp180_get_cal_param;
	bmp->read_reg 		= bmp180_read_reg;
	bmp->write_reg 		= bmp180_write_reg;
	bmp->get_cal_param 	= bmp180_get_cal_param;
	bmp->get_ut 		= bmp180_get_ut;
	bmp->get_up 		= bmp180_get_up;
	bmp->read_data 		= bmp180_read_data;
	return 0;
}



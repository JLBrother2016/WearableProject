#ifndef _I2C_H_
#define _I2C_H_

#include <stm32f4xx.h>
#include <rtthread.h>
#include "pin.h"

#define I2C_ACK 	0
#define I2C_NACK 	1
#define I2C_WR		0		/* 写控制bit */
#define I2C_RD		1		/* 读控制bit */

/* I2C pin structure */
typedef struct i2c_pin i2c_pin_t;
struct i2c_pin {
	pin_t 			scl;		
	pin_t 			sda;
};

/* i2c bus structure */
typedef struct i2c_bus i2c_bus_t;
struct i2c_bus {
	i2c_pin_t 					pin;

	int32_t (*init)(i2c_bus_t *dev);
	int32_t (*start)(i2c_bus_t *dev);
	int32_t (*restart)(i2c_bus_t *dev);
	int32_t (*stop)(i2c_bus_t *dev);
	int32_t (*send_byte)(i2c_bus_t *dev, uint8_t val);
	int32_t (*recv_byte)(i2c_bus_t *dev, uint8_t *val);
	int32_t (*send_nbytes)(i2c_bus_t *dev, uint8_t n, uint8_t *val);
	int32_t (*recv_nbytes)(i2c_bus_t *dev, uint8_t n, uint8_t *val);
	int32_t (*wait_ack)(i2c_bus_t *dev);
	int32_t (*send_ack)(i2c_bus_t *dev);
	int32_t (*send_nack)(i2c_bus_t *dev);
};

int32_t i2c_bus_register(i2c_bus_t *dev, GPIO_TypeDef *port, uint32_t scl, uint32_t sda);
#endif /* _I2C_H_ */

#ifndef _BMP180_H_
#define _BMP180_H_

#include "i2c.h"

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct bmp180 bmp180_t;
struct bmp180 {
	i2c_bus_t 					*i2c;
	int16_t 					ac1;
	int16_t 					ac2;
	int16_t 					ac3;
	uint16_t 					ac4;
	uint16_t 					ac5;
	uint16_t 					ac6;
	int16_t 					b1;
	int16_t 					b2;
	int16_t 					mb;
	int16_t 					mc;
	int16_t 					md;
	int32_t 					ut;
	int32_t 					up;
	int32_t 					temperature;
	int32_t 					pressure;

	int32_t (*check_device)(bmp180_t *bmp);
	int32_t (*init)(bmp180_t *bmp);
	int32_t (*read_reg)(bmp180_t *bmp, uint8_t reg, void *val);
	int32_t (*write_reg)(bmp180_t *bmp, uint8_t reg, uint8_t val);
	int32_t (*get_cal_param)(bmp180_t *bmp);
	int32_t (*get_ut)(bmp180_t *bmp);
	int32_t (*get_up)(bmp180_t *bmp);
	int32_t (*read_data)(bmp180_t *bmp);	
};

extern bmp180_t bmp180;
int32_t bmp180_register(bmp180_t *bmp, i2c_bus_t *i2c);

#ifdef __cplusplus
}
#endif

#endif

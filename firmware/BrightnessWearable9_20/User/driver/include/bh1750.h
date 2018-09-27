#ifndef _BH1750_H_
#define _BH1750_H_

#include "i2c.h"

#ifdef __cplusplus
	extern "C" {
#endif

#define BH1750_CMD_SET_MEASURE_MODE 		1
#define BH1750_CMD_SET_SENSITIVITY 			2

typedef enum
{
	CONT_H_MODE 	= 1,
	CONT_H_MODE2 	= 2,
	CONT_L_MODE		= 3
} bh1750_measure_mode_t;


typedef struct bh1750 bh1750_t;
struct bh1750
{
	i2c_bus_t *i2c;
	int32_t (*check_device)(bh1750_t *dev);
	int32_t (*init)(bh1750_t *dev);
	int32_t (*read_data)(bh1750_t *dev);
	int32_t (*get_lx)(bh1750_t *dev);
	int32_t (*write_cmd)(bh1750_t *dev, uint8_t cmd);
	int32_t (*ioctl)(bh1750_t *dev, uint8_t cmd, int32_t val);
	
	
	bh1750_measure_mode_t mode;			/* 测量模式 */
	uint8_t mtreg;							/* 灵敏度倍率 */
	float lx;								/* 光线强度 */
	uint8_t buffer[2];
};

extern bh1750_t bh1750;
int32_t bh1750_register(bh1750_t *dev, i2c_bus_t *i2c);

#ifdef __cplusplus
}
#endif

#endif

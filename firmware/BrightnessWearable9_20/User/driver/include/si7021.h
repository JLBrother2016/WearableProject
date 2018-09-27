#ifndef _SI7021_H_
#define _SI7021_H_

#include "i2c.h"
#include <rtthread.h>

#ifdef __cplusplus
	extern "C" {
#endif
		
typedef struct si7021 si7021_t;
struct si7021 {
	i2c_bus_t 					*i2c;
	uint16_t 					temperature;
	uint16_t 					humidity;
	rt_mutex_t 				mutex;
	int32_t (*check_device)(si7021_t *dev);
	int32_t (*init)(si7021_t *dev);
	int32_t (*read_data)(si7021_t *dev);
	
};

extern si7021_t  	si7021;	
int32_t si7021_register(si7021_t *dev, i2c_bus_t *i2c);

#ifdef __cplusplus
}
#endif

#endif

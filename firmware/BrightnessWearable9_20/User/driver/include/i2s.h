#ifndef _I2S_H_
#define _I2S_H_

#include <stm32f4xx.h>
#include "dma.h"

/*
	I2S_WS			PB12
	I2SCK			PB13
	I2Sext_SD		PC2
	I2S_SD			PC3
	I2S_MCK			PC6
*/

void i2s_gpio_config(void);
void i2s_set_rx_mode(uint16_t i2s_data_format);
void i2s_set_tx_mode(uint16_t i2s_data_format);
int32_t i2s_set_sample_rate(uint32_t sr);


#endif

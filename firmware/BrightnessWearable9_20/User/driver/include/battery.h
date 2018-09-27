#ifndef _BATTERY_H_
#define _BATTERY_H_

#include <stm32f4xx.h>

void battery_adc_init(void);
uint16_t get_battery_adc_val(uint8_t ch);

#endif

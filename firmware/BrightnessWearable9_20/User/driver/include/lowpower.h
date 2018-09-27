#ifndef _LOWPOWER_H_
#define _LOWPOWER_H_

#include <stm32f4xx.h>
#include <rtthread.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void enter_standby_mode(void);
extern void enter_stop_mode(void);
extern void leave_stop_mode(void);
extern void set_nouse_gpio_an_mode(void);
	
	
#ifdef __cplusplus
}
#endif
	
#endif

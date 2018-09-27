#ifndef _WATCHDOG_H_
#define _WATCHDOG_H_

#include <stm32f4xx.h>

#ifdef __cplusplus
extern "C" {
#endif
	
extern void watchdog_init_sec(uint8_t sec);
extern void watchdog_feed(void);
	
#ifdef __cplusplus
}
#endif


#endif

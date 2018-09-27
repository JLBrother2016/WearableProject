#ifndef _LED_H_
#define _LED_H_

#include "pin.h"

/* LED 控制块 */
typedef struct led led_t;
struct led {	
	int32_t (*on)(led_t *led);			/* LED 点亮 */
	int32_t (*off)(led_t *led);			/* LED 熄灭 */
	int32_t (*toggle)(led_t *led);		/* LED 翻转 */
	
	pin_t 	pin;						/* 包含pin控制块 */					
};

int32_t led_register(led_t *led, GPIO_TypeDef *port, uint32_t num);

#endif /* _LED_H_ */

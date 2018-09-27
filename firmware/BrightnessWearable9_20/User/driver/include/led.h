#ifndef _LED_H_
#define _LED_H_

#include "pin.h"

/* LED ���ƿ� */
typedef struct led led_t;
struct led {	
	int32_t (*on)(led_t *led);			/* LED ���� */
	int32_t (*off)(led_t *led);			/* LED Ϩ�� */
	int32_t (*toggle)(led_t *led);		/* LED ��ת */
	
	pin_t 	pin;						/* ����pin���ƿ� */					
};

int32_t led_register(led_t *led, GPIO_TypeDef *port, uint32_t num);

#endif /* _LED_H_ */

#ifndef _KEY_H_
#define _KEY_H_

#include "pin.h"

typedef enum
{
	KEY_IDLE = 0,
	SINGLE_CLICK = 1,
	DOUBLE_CLICK = 2,
	LONG_PRESS = 3
} key_event_t;


/* °´¼ü¿ØÖÆ¿é */
typedef struct key key_t;
struct key {
	pin_t pin;
	
	int32_t (*is_key_down)(key_t *key);
};

void exti_init(void);
int32_t key_register(key_t *key, GPIO_TypeDef *port, uint32_t pin_number);
int32_t key_register_pu(key_t *key, GPIO_TypeDef *port, uint32_t pin_number);
void keys_scan(key_t *key[], key_event_t key_event[]);
#endif /* _KEY_H_ */

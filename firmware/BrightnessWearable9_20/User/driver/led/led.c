#include "led.h"
#include "config.h"


/* 引脚低电平点亮LED */
int32_t led_on(led_t *led)
{
	pin_t *pin = &led->pin;

	assert(led);
	pin->clr(pin);
	return 0;
}

/* 引脚高电平熄灭LED */
int32_t led_off(led_t *led)
{
	pin_t *pin = &led->pin;
	
	assert(led);
	pin->set(pin);
	return 0;
}

/* LED 翻转 */
int32_t led_toggle(led_t *led)
{
	pin_t *pin = &led->pin;
	
	assert(led);
	pin->port->ODR ^= pin->num;
	return 0;
}

/* 注册一个LED ， 指定端口， 引脚 */
int32_t led_register(led_t *led, GPIO_TypeDef *port, uint32_t num)
{
	assert(led);
	if (pin_register(&led->pin, port, num, OUT, PP, NP) != 0) {
		err("led_register failed!\r\n");
		return -1;
	}
	led->on     			= led_on;
	led->off    			= led_off;
	led->toggle 			= led_toggle;

	return 0;
}


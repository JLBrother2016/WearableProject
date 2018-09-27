#include "pin.h"
#include "config.h"

/* GPIO 初始化 */
static void pin_init(pin_t *pin)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(pin->rcc, ENABLE);							/* 开启GPIO时钟 */
	GPIO_InitStructure.GPIO_Mode 			= pin->mode;				/* 模式 */
	GPIO_InitStructure.GPIO_OType 			= pin->type;				/* 输出模式 */
	GPIO_InitStructure.GPIO_PuPd 			= pin->pupd;				/* 上拉下拉 */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;			/* 速度 */
	GPIO_InitStructure.GPIO_Pin 			= pin->num;					/* 引脚 */
	GPIO_Init(pin->port, &GPIO_InitStructure);							/* 初始化指定引脚 */
}

/* 将对应的引脚置1 */
int32_t pin_set(pin_t *pin)
{
	assert(pin);
	pin->port->BSRRL = pin->num;
	return 0;
}

/* 将对应的引脚清0 */
int32_t pin_clr(pin_t *pin)
{
	assert(pin);
	pin->port->BSRRH = pin->num;
	return 0;
}

/* 读取引脚的状态 */
int32_t pin_get(pin_t *pin)
{
	assert(pin);
	return (pin->port->IDR & pin->num);
}

/* 注册引脚 */
int32_t pin_register(pin_t *pin, GPIO_TypeDef *port, uint32_t pin_num, 
							GPIOMode_TypeDef mode, GPIOOType_TypeDef type, 
							GPIOPuPd_TypeDef pupd)
{	
	assert(pin);
	if(port == GPIOA)
		pin->rcc = RCC_AHB1Periph_GPIOA;
	else if(port == GPIOB)
		pin->rcc = RCC_AHB1Periph_GPIOB;
	else if(port == GPIOC)
		pin->rcc = RCC_AHB1Periph_GPIOC;
	else if(port == GPIOD)
		pin->rcc = RCC_AHB1Periph_GPIOD;
	else if(port == GPIOE)
		pin->rcc = RCC_AHB1Periph_GPIOE;
	else if(port == GPIOF)
		pin->rcc = RCC_AHB1Periph_GPIOF;
	
	pin->port 				= port;
	pin->num				= 1 << pin_num;
	pin->mode 				= mode;
	pin->type 				= type;
	pin->pupd 				= pupd;
	pin->set   				= pin_set;
	pin->clr 				= pin_clr;
	pin->get   				= pin_get;
	pin_init(pin);									/* 注册完就立刻调用初始化函数 */

	return 0;
}


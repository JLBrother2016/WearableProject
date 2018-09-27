#include "pin.h"
#include "config.h"

/* GPIO ��ʼ�� */
static void pin_init(pin_t *pin)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(pin->rcc, ENABLE);							/* ����GPIOʱ�� */
	GPIO_InitStructure.GPIO_Mode 			= pin->mode;				/* ģʽ */
	GPIO_InitStructure.GPIO_OType 			= pin->type;				/* ���ģʽ */
	GPIO_InitStructure.GPIO_PuPd 			= pin->pupd;				/* �������� */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_50MHz;			/* �ٶ� */
	GPIO_InitStructure.GPIO_Pin 			= pin->num;					/* ���� */
	GPIO_Init(pin->port, &GPIO_InitStructure);							/* ��ʼ��ָ������ */
}

/* ����Ӧ��������1 */
int32_t pin_set(pin_t *pin)
{
	assert(pin);
	pin->port->BSRRL = pin->num;
	return 0;
}

/* ����Ӧ��������0 */
int32_t pin_clr(pin_t *pin)
{
	assert(pin);
	pin->port->BSRRH = pin->num;
	return 0;
}

/* ��ȡ���ŵ�״̬ */
int32_t pin_get(pin_t *pin)
{
	assert(pin);
	return (pin->port->IDR & pin->num);
}

/* ע������ */
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
	pin_init(pin);									/* ע��������̵��ó�ʼ������ */

	return 0;
}


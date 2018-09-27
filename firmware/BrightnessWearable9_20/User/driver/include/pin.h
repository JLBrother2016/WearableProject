#ifndef _PIN_H_
#define _PIN_H_

#include <stm32f4xx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO ģʽ */
#define 	IN			GPIO_Mode_IN 		/* ����ģʽ */	
#define 	OUT			GPIO_Mode_OUT		/* ���ģʽ */	
#define 	AF			GPIO_Mode_AF		/* ���ù���ģʽ */	
#define 	AN			GPIO_Mode_AN		/* ģ������ģʽ */

/* GPIO ���ģʽ */
#define    	PP 	 		GPIO_OType_PP		/* ������� */
#define 	OD			GPIO_OType_OD		/* ��©��� */

/* GPIO ������������ */
#define 	NP			GPIO_PuPd_NOPULL	/* �������������� */
#define 	PU			GPIO_PuPd_UP		/* ���� */
#define 	PD		  	GPIO_PuPd_DOWN		/* ���� */


/* ���ٲ������� */
#define PIN_SET(pin)				pin->port->BSRRL 	= pin->num
#define PIN_CLR(pin)				pin->port->BSRRH 	= pin->num
#define PIN_GET(pin)				pin->port->IDR 		& pin->num


/* GPIO ���ſ��ƿ� */
typedef struct pin pin_t;
struct pin {
	int32_t (*set)(pin_t *pin);					/* ��������Ϊ�ߵ�ƽ */
	int32_t (*clr)(pin_t *pin);					/* ��������Ϊ�͵�ƽ */
	int32_t (*get)(pin_t *pin);					/* ���Ŷ�ȡ */

	GPIO_TypeDef 			*port;					/* GPIO �˿� */
	uint32_t 			 	rcc;					/* GPIO ʱ�� */
	uint32_t 			 	num;					/* GPIO ���� */
	GPIOMode_TypeDef 	 	mode;					/* GPIO ģʽ */							
	GPIOOType_TypeDef  	    type;					/* GPIO ���ģʽ */
	GPIOPuPd_TypeDef 	 	pupd;					/* GPIO ��������ģʽ */
};

int32_t pin_register(pin_t *pin, GPIO_TypeDef *port, uint32_t num, 
							GPIOMode_TypeDef mode, GPIOOType_TypeDef type, GPIOPuPd_TypeDef pupd);

#ifdef __cplusplus
}
#endif


#endif /* _PIN_H_ */

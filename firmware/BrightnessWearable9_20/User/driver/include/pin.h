#ifndef _PIN_H_
#define _PIN_H_

#include <stm32f4xx.h>

#ifdef __cplusplus
extern "C" {
#endif

/* GPIO 模式 */
#define 	IN			GPIO_Mode_IN 		/* 输入模式 */	
#define 	OUT			GPIO_Mode_OUT		/* 输出模式 */	
#define 	AF			GPIO_Mode_AF		/* 复用功能模式 */	
#define 	AN			GPIO_Mode_AN		/* 模拟输入模式 */

/* GPIO 输出模式 */
#define    	PP 	 		GPIO_OType_PP		/* 推挽输出 */
#define 	OD			GPIO_OType_OD		/* 开漏输出 */

/* GPIO 上拉下拉配置 */
#define 	NP			GPIO_PuPd_NOPULL	/* 无上拉或者下拉 */
#define 	PU			GPIO_PuPd_UP		/* 上拉 */
#define 	PD		  	GPIO_PuPd_DOWN		/* 下拉 */


/* 快速操作引脚 */
#define PIN_SET(pin)				pin->port->BSRRL 	= pin->num
#define PIN_CLR(pin)				pin->port->BSRRH 	= pin->num
#define PIN_GET(pin)				pin->port->IDR 		& pin->num


/* GPIO 引脚控制块 */
typedef struct pin pin_t;
struct pin {
	int32_t (*set)(pin_t *pin);					/* 引脚设置为高电平 */
	int32_t (*clr)(pin_t *pin);					/* 引脚设置为低电平 */
	int32_t (*get)(pin_t *pin);					/* 引脚读取 */

	GPIO_TypeDef 			*port;					/* GPIO 端口 */
	uint32_t 			 	rcc;					/* GPIO 时钟 */
	uint32_t 			 	num;					/* GPIO 引脚 */
	GPIOMode_TypeDef 	 	mode;					/* GPIO 模式 */							
	GPIOOType_TypeDef  	    type;					/* GPIO 输出模式 */
	GPIOPuPd_TypeDef 	 	pupd;					/* GPIO 上拉下拉模式 */
};

int32_t pin_register(pin_t *pin, GPIO_TypeDef *port, uint32_t num, 
							GPIOMode_TypeDef mode, GPIOOType_TypeDef type, GPIOPuPd_TypeDef pupd);

#ifdef __cplusplus
}
#endif


#endif /* _PIN_H_ */

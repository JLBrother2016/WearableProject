#include "key.h"
#include "config.h"
#include "lowpower.h"

extern uint8_t stop_flag;

/* PC0映射到EXTI0 */
void exti_init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);			//使能SYSCFG时钟
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);	//PA0 连接到中断线0
	
	/* 配置EXTI_Line0 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;						//LINE0
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;				//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 			//上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;						//使能LINE0
	EXTI_Init(&EXTI_InitStructure);									//配置
 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;				//外部中断0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;			//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);
}

/* 
 * 外部中断0服务程序, 用来将CPU从STOP模式唤醒 
 */
void EXTI0_IRQHandler(void)
{
//	rt_interrupt_enter();
	leave_stop_mode();
	EXTI_ClearITPendingBit(EXTI_Line0); //清除LINE0上的中断标志位
//	rt_interrupt_leave();
}

int32_t is_key_down(key_t *key)
{
	pin_t *pin = &key->pin;
	assert(key);
	return (pin->port->IDR & pin->num)? 0 : 1;
}

/* key引脚没有配置为内部上拉，需要外部上拉电阻 */
int32_t key_register(key_t *key, GPIO_TypeDef *port, uint32_t pin_num)
{
	assert(key);
	key->is_key_down = is_key_down;
	if (pin_register(&key->pin, port, pin_num, IN, PP, NP) != 0) {		/* 没有配置上拉和下拉 */
		err("key_register failed!\r\n");
		return -1;
	}
	return 0;
}

/* key 引脚配置为内部上拉，不需要外部上拉电阻 */
int32_t key_register_pu(key_t *key, GPIO_TypeDef *port, uint32_t pin_num)
{
	assert(key);
	key->is_key_down = is_key_down;
	if (pin_register(&key->pin, port, pin_num, IN, PP, PU) != 0) {		/* 配置为下拉 */
		err("key_register_pu failed!\r\n");
		return -1;
	}
	return 0;
}

#define KEY_NUM     2			/* 一共2个按键 */
#define KEY_NOM     0			/* 按键空闲状态 */
#define KEY_1DOW    1			/* 按键一次按下 */
#define KEY_1UP     2			/* 按键一次抬起 */
#define KEY_1LNG    3			/* 按键一次持续按下 */
#define KEY_2DOW    4			/* 按键连续两次按下 */
#define KEY_2UP     5			/* 按键连续两次按下后抬起 */
#define KEY_2LNG    6			/* 按键连续两次按下并持续按下 */
void keys_scan(key_t *key[], key_event_t key_event[])
{
	int i;
	static uint8_t delay0[KEY_NUM] = {0, 0};
	static uint8_t delay[KEY_NUM]  = {0, 0};
	static uint8_t status[KEY_NUM] = {0, 0};
	for (i = 0; i < KEY_NUM; i++) {
		key_event[i] = KEY_IDLE;
		switch(status[i]) {
		case KEY_NOM:
			if (!(key[i]->is_key_down(key[i]))) {
				delay[i]  = 0;
				delay0[i] = 0;
			} else {
				delay[i]++;
				if (delay[i]>=3) {
					delay[i] = 0;
					status[i] = KEY_1DOW;
				}
			}
			break;
				
		case KEY_1DOW:
			if (!(key[i]->is_key_down(key[i]))) {
				delay0[i]++;
				if (delay0[i]>=2) {
					delay0[i] = 0;
					delay[i]  = 0;
					status[i] = KEY_1UP;
				}
			} else {
				delay0[i] = 0;
				delay[i]++;
				if(delay[i] >= 20) 
				{
					status[i] = KEY_1LNG;
					delay[i] = 0;
					key_event[i] = LONG_PRESS;
				}
			}
			break;
				
		case KEY_1UP:
			if (!(key[i]->is_key_down(key[i]))) {
				delay0[i]++;
				if (delay0[i] > 20) {
					delay0[i] = 0;
					delay[i]  = 0;
					key_event[i] = SINGLE_CLICK;
					status[i] = KEY_NOM;
				}
			} else {
				delay[i]++;
				if(delay[i]>=2) 
				{
					delay[i]  = 0;
					delay0[i] = 0;
					status[i] = KEY_2DOW;
				}
			}
			break;
				
		case KEY_1LNG:
			if (!(key[i]->is_key_down(key[i]))) {
				delay0[i]++;
				if (delay0[i] >= 2) {
					delay0[i] = 0;
					delay[i]  = 0;
					key_event[i] = KEY_IDLE;
					status[i] = KEY_NOM;
				}
			} else
				delay[i]++;
			break;
				
		case KEY_2DOW:
			if(!(key[i]->is_key_down(key[i]))) {
				delay0[i]++;
				if(delay0[i]>=2) {
					delay0[i] = 0;
					delay[i]  = 0;
					status[i] = KEY_2UP;
					key_event[i] = DOUBLE_CLICK;
				}
			} else {
				delay0[i] = 0;
				delay[i]++;
				if(delay[i] >= 20) 
				{
					status[i] = KEY_2LNG;
					delay[i]  = 0;
					key_event[i] = LONG_PRESS;
					break;
				}
			}
			break;
				
		case KEY_2UP:
			delay[i]  = 0;
			delay0[i] = 0;
			status[i] = KEY_NOM;
			break; 
			
		case KEY_2LNG:
			if(!(key[i]->is_key_down(key[i]))) {
				delay0[i]++;
				if(delay0[i]>=2) {
					delay0[i] = 0;
					delay[i]  = 0;
					status[i] = KEY_NOM;
				}
			} else 
				delay[i]++;
			break;  
				
		default:
			break;
	    }
	}
}

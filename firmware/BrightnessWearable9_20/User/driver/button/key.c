#include "key.h"
#include "config.h"
#include "lowpower.h"

extern uint8_t stop_flag;

/* PC0ӳ�䵽EXTI0 */
void exti_init(void)
{
	NVIC_InitTypeDef   NVIC_InitStructure;
	EXTI_InitTypeDef   EXTI_InitStructure;
 
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);			//ʹ��SYSCFGʱ��
 
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOC, EXTI_PinSource0);	//PA0 ���ӵ��ж���0
	
	/* ����EXTI_Line0 */
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;						//LINE0
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;				//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 			//�����ش��� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;						//ʹ��LINE0
	EXTI_Init(&EXTI_InitStructure);									//����
 
	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;				//�ⲿ�ж�0
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;			//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;					//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);
}

/* 
 * �ⲿ�ж�0�������, ������CPU��STOPģʽ���� 
 */
void EXTI0_IRQHandler(void)
{
//	rt_interrupt_enter();
	leave_stop_mode();
	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ
//	rt_interrupt_leave();
}

int32_t is_key_down(key_t *key)
{
	pin_t *pin = &key->pin;
	assert(key);
	return (pin->port->IDR & pin->num)? 0 : 1;
}

/* key����û������Ϊ�ڲ���������Ҫ�ⲿ�������� */
int32_t key_register(key_t *key, GPIO_TypeDef *port, uint32_t pin_num)
{
	assert(key);
	key->is_key_down = is_key_down;
	if (pin_register(&key->pin, port, pin_num, IN, PP, NP) != 0) {		/* û���������������� */
		err("key_register failed!\r\n");
		return -1;
	}
	return 0;
}

/* key ��������Ϊ�ڲ�����������Ҫ�ⲿ�������� */
int32_t key_register_pu(key_t *key, GPIO_TypeDef *port, uint32_t pin_num)
{
	assert(key);
	key->is_key_down = is_key_down;
	if (pin_register(&key->pin, port, pin_num, IN, PP, PU) != 0) {		/* ����Ϊ���� */
		err("key_register_pu failed!\r\n");
		return -1;
	}
	return 0;
}

#define KEY_NUM     2			/* һ��2������ */
#define KEY_NOM     0			/* ��������״̬ */
#define KEY_1DOW    1			/* ����һ�ΰ��� */
#define KEY_1UP     2			/* ����һ��̧�� */
#define KEY_1LNG    3			/* ����һ�γ������� */
#define KEY_2DOW    4			/* �����������ΰ��� */
#define KEY_2UP     5			/* �����������ΰ��º�̧�� */
#define KEY_2LNG    6			/* �����������ΰ��²��������� */
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

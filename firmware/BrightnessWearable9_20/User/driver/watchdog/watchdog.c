#include "watchdog.h"


/*
 * �������Ź���ʼ�� sec������Ҫιһ�ι�
 * prer:��Ƶ��:0~7(ֻ�е�3λ��Ч!)
 * rlr:�Զ���װ��ֵ,0~0XFFF.
 * ��Ƶ����=4*2^prer.�����ֵֻ����256!
 * rlr:��װ�ؼĴ���ֵ:��11λ��Ч
 * ʱ�����(���):Tout=((4*2^prer)*rlr) / 32 (ms).
 */
void watchdog_init_sec(uint8_t sec)
{
	IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable); //ʹ�ܶ�IWDG->PR IWDG->RLR��д
	IWDG_SetPrescaler(4); 			//����IWDG��Ƶϵ�� prer=4
	IWDG_SetReload(500*sec);   		//����IWDGװ��ֵ rlr=500
	IWDG_ReloadCounter(); 			//reload
	IWDG_Enable();       			//ʹ�ܿ��Ź�
}

/*
 * ι��
 */
void watchdog_feed(void)
{
	IWDG_ReloadCounter();		//reload
}

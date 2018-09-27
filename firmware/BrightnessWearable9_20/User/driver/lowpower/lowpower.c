#include "lowpower.h"
#include "config.h"

extern uint8_t stop_flag;
	
void enter_standby_mode(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//ʹ��PWRʱ��
	PWR_BackupAccessCmd(ENABLE);							//���������ʹ��
	
	//�ر����RTC�ж�
	RTC_ITConfig(RTC_IT_TS | RTC_IT_WUT | RTC_IT_ALRB | RTC_IT_ALRA, DISABLE);	//�ر�RTC����жϡ�
	RTC_ClearITPendingBit(RTC_IT_TS|RTC_IT_WUT|RTC_IT_ALRB|RTC_IT_ALRA);//���RTC����жϱ�־λ��
	PWR_ClearFlag(PWR_FLAG_WU);		//��� Wake-up ��־
//	PWR_WakeUpPinCmd(ENABLE);		//����WKUP���ڻ���
	PWR_EnterSTANDBYMode();			//�������ģʽ
}

//�͹��ģ�û�õ���������Ϊģ������
void set_all_gpio_an_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

//��û��ʹ�õ�GPIO����Ϊģ������
//PA6 PA4 PA15 PA8 PB5 PB4 PB14 PB15
void set_nouse_gpio_an_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_4 | GPIO_Pin_8 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_4 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}


/* 
 * ��CPU����STOP
 * ע���STOPģʽ����Ҫ��������ʱ�� 
 */
void enter_stop_mode(void)
{
	debug("����GPIOȫ������Ϊģ������");
	set_all_gpio_an_mode();
	debug("CPU���ڽ���STOPģʽ\r\n");
	stop_flag = 1;
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); //����ֹͣģʽ���ں�ֹͣ���������蹤����
}


/*
 * CPU��STOPģʽ�������ú������ܱ�����CPU���жϺ�������
*/
void leave_stop_mode(void)
{
	if (stop_flag == 1) {
		stop_flag = 0;
		SystemInit(); //��������ʱ��
		debug("CPU�Ѿ�������\r\n");
	}
}


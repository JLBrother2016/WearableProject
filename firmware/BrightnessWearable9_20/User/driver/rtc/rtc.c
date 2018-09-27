#include "rtc.h"
#include "config.h"
#include "watchdog.h"


RTC_TimeTypeDef  RTC_time;
RTC_DateTypeDef  RTC_date;

/*
 * RTCʱ������
 * ampm:����AM����PM
 * #define RTC_H12_AM                     ((uint8_t)0x00)
 * #define RTC_H12_PM                     ((uint8_t)0x40)
 */
int32_t RTC_set_time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm)
{
	RTC_TimeTypeDef 						RTC_TimeTypeInitStructure;
	
	RTC_TimeTypeInitStructure.RTC_Hours 	= hour;
	RTC_TimeTypeInitStructure.RTC_Minutes 	= min;
	RTC_TimeTypeInitStructure.RTC_Seconds 	= sec;
	RTC_TimeTypeInitStructure.RTC_H12 		= ampm;

	if (RTC_SetTime(RTC_Format_BIN, &RTC_TimeTypeInitStructure) == ERROR) {
		rt_kprintf("RTC_set_time error!\r\n");
		return -1;
	}	
	
	rt_kprintf("RTC set time OK!\r\n");
	return 0;
}


/*
 * RTC ��������
 */
int32_t RTC_set_date(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday)
{
	RTC_DateTypeDef 						RTC_DateTypeInitStructure;
	
	RTC_DateTypeInitStructure.RTC_Date		= day;
	RTC_DateTypeInitStructure.RTC_Month		= month;
	RTC_DateTypeInitStructure.RTC_WeekDay	= weekday;
	RTC_DateTypeInitStructure.RTC_Year		= year;
	if (RTC_SetDate(RTC_Format_BIN, &RTC_DateTypeInitStructure) == ERROR) {
		rt_kprintf("RTC_set_date error!\r\n");
		return -1;
	}
	rt_kprintf("����ʱ��: %s %s\n", __DATE__, __TIME__);
	rt_kprintf("RTC set date OK!\r\n");
	return 0;
}


/*
 * RTC ��ȡʱ�������
 */
int32_t get_rtc_info(rtc_t *info)
{
	RTC_TimeTypeDef 			time;
	RTC_DateTypeDef 			date;
	
	RTC_GetDate(RTC_Format_BIN, &date);	/* ��ȡ���� */
	RTC_GetTime(RTC_Format_BIN, &time);	/* ��ȡʱ�� */
	
	info->year = date.RTC_Year + 2000;
	info->month = date.RTC_Month;
	info->day = date.RTC_Date;
	info->weekday = date.RTC_WeekDay;
	info->hour = time.RTC_Hours;
	info->min = time.RTC_Minutes;
	info->sec = time.RTC_Seconds;
	
	return 0;
}

/* 
 * RTC ��ʼ��
 */
int32_t RTC_init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	uint32_t retry = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);				/* ʹ��PWRʱ�� */
	PWR_BackupAccessCmd(ENABLE);									/* ʹ�ܺ󱸼Ĵ������� */
	
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)	{			/* �Ƿ��һ������ */
		RCC_LSEConfig(RCC_LSE_ON);									/* LSE ���� */   
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	{		/* ���ָ����RCC��־λ�������,�ȴ����پ������ */
			retry++;
			delay_10ms(10);
		}
		if (retry >= 300) {
			rt_kprintf("RTC ����ʧ��!\r\n");
			return -1;												/* LSE ����ʧ��. */ 
		}
		rt_kprintf("32.768K�������!\r\n");
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);						/* ����RTCʱ��(RTCCLK),ѡ��LSE��ΪRTCʱ�� */   
		RCC_RTCCLKCmd(ENABLE);										/* ʹ��RTCʱ�� */

		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;					/* RTC�첽��Ƶϵ��(1~0X7F) */
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;					/* RTCͬ����Ƶϵ��(0~7FFF) */
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;		/* RTC����Ϊ,24Сʱ��ʽ */
		RTC_Init(&RTC_InitStructure);
 
		RTC_set_time(8, 00, 00, RTC_H12_AM);						/* ����ʱ�� */
		RTC_set_date(17, 9, 21, 1);									/* �������� */
	 
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x5050);				/* ����Ѿ���ʼ������ */
	} 
 
	return 0;
}

/* 
 * RTC �������ڻ���ʱ����1s 
 */
int32_t RTC_set_wakeup_1s(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	RTC_WakeUpCmd(DISABLE);										//�ر�WAKE UP
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);		//����ʱ��ѡ��
	RTC_SetWakeUpCounter(0);									//����WAKE UP�Զ���װ�ؼĴ���
	RTC_ClearITPendingBit(RTC_IT_WUT); 							//���RTC WAKE UP�ı�־
	EXTI_ClearITPendingBit(EXTI_Line22);						//���LINE22�ϵ��жϱ�־λ 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);							//����WAKE UP ��ʱ���ж�
	RTC_WakeUpCmd( ENABLE);										//����WAKE UP ��ʱ��
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;					//LINE22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//�ж��¼�
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 		//�����ش��� 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//ʹ��LINE22
	EXTI_Init(&EXTI_InitStructure);								//����
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//��ռ���ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//ʹ���ⲿ�ж�ͨ��
	NVIC_Init(&NVIC_InitStructure);								//����
	
	return 0;
}

//RTC WAKE UP�жϷ�����
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF) == SET)//WK_UP�ж�?
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);	       //����жϱ�־
		watchdog_feed();
	}   
	EXTI_ClearITPendingBit(EXTI_Line22);      //����ж���22���жϱ�־ 								
}

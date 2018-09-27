#include "rtc.h"
#include "config.h"
#include "watchdog.h"


RTC_TimeTypeDef  RTC_time;
RTC_DateTypeDef  RTC_date;

/*
 * RTC时间设置
 * ampm:设置AM或者PM
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
 * RTC 日期设置
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
	rt_kprintf("编译时间: %s %s\n", __DATE__, __TIME__);
	rt_kprintf("RTC set date OK!\r\n");
	return 0;
}


/*
 * RTC 获取时间和日期
 */
int32_t get_rtc_info(rtc_t *info)
{
	RTC_TimeTypeDef 			time;
	RTC_DateTypeDef 			date;
	
	RTC_GetDate(RTC_Format_BIN, &date);	/* 获取日期 */
	RTC_GetTime(RTC_Format_BIN, &time);	/* 获取时间 */
	
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
 * RTC 初始化
 */
int32_t RTC_init(void)
{
	RTC_InitTypeDef RTC_InitStructure;
	uint32_t retry = 0;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);				/* 使能PWR时钟 */
	PWR_BackupAccessCmd(ENABLE);									/* 使能后备寄存器访问 */
	
	if (RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x5050)	{			/* 是否第一次配置 */
		RCC_LSEConfig(RCC_LSE_ON);									/* LSE 开启 */   
		while (RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)	{		/* 检查指定的RCC标志位设置与否,等待低速晶振就绪 */
			retry++;
			delay_10ms(10);
		}
		if (retry >= 300) {
			rt_kprintf("RTC 设置失败!\r\n");
			return -1;												/* LSE 开启失败. */ 
		}
		rt_kprintf("32.768K晶振就绪!\r\n");
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);						/* 设置RTC时钟(RTCCLK),选择LSE作为RTC时钟 */   
		RCC_RTCCLKCmd(ENABLE);										/* 使能RTC时钟 */

		RTC_InitStructure.RTC_AsynchPrediv = 0x7F;					/* RTC异步分频系数(1~0X7F) */
		RTC_InitStructure.RTC_SynchPrediv  = 0xFF;					/* RTC同步分频系数(0~7FFF) */
		RTC_InitStructure.RTC_HourFormat   = RTC_HourFormat_24;		/* RTC设置为,24小时格式 */
		RTC_Init(&RTC_InitStructure);
 
		RTC_set_time(8, 00, 00, RTC_H12_AM);						/* 设置时间 */
		RTC_set_date(17, 9, 21, 1);									/* 设置日期 */
	 
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x5050);				/* 标记已经初始化过了 */
	} 
 
	return 0;
}

/* 
 * RTC 设置周期唤醒时间间隔1s 
 */
int32_t RTC_set_wakeup_1s(void)
{
	EXTI_InitTypeDef   EXTI_InitStructure;
	NVIC_InitTypeDef   NVIC_InitStructure;
	
	RTC_WakeUpCmd(DISABLE);										//关闭WAKE UP
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);		//唤醒时钟选择
	RTC_SetWakeUpCounter(0);									//设置WAKE UP自动重装载寄存器
	RTC_ClearITPendingBit(RTC_IT_WUT); 							//清除RTC WAKE UP的标志
	EXTI_ClearITPendingBit(EXTI_Line22);						//清除LINE22上的中断标志位 
	RTC_ITConfig(RTC_IT_WUT,ENABLE);							//开启WAKE UP 定时器中断
	RTC_WakeUpCmd( ENABLE);										//开启WAKE UP 定时器
	
	EXTI_InitStructure.EXTI_Line = EXTI_Line22;					//LINE22
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;			//中断事件
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; 		//上升沿触发 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;					//使能LINE22
	EXTI_Init(&EXTI_InitStructure);								//配置
	
	NVIC_InitStructure.NVIC_IRQChannel = RTC_WKUP_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;//抢占优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//使能外部中断通道
	NVIC_Init(&NVIC_InitStructure);								//配置
	
	return 0;
}

//RTC WAKE UP中断服务函数
void RTC_WKUP_IRQHandler(void)
{    
	if(RTC_GetFlagStatus(RTC_FLAG_WUTF) == SET)//WK_UP中断?
	{ 
		RTC_ClearFlag(RTC_FLAG_WUTF);	       //清除中断标志
		watchdog_feed();
	}   
	EXTI_ClearITPendingBit(EXTI_Line22);      //清除中断线22的中断标志 								
}

#include "lowpower.h"
#include "config.h"

extern uint8_t stop_flag;
	
void enter_standby_mode(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//使能PWR时钟
	PWR_BackupAccessCmd(ENABLE);							//后备区域访问使能
	
	//关闭相关RTC中断
	RTC_ITConfig(RTC_IT_TS | RTC_IT_WUT | RTC_IT_ALRB | RTC_IT_ALRA, DISABLE);	//关闭RTC相关中断。
	RTC_ClearITPendingBit(RTC_IT_TS|RTC_IT_WUT|RTC_IT_ALRB|RTC_IT_ALRA);//清除RTC相关中断标志位。
	PWR_ClearFlag(PWR_FLAG_WU);		//清除 Wake-up 标志
//	PWR_WakeUpPinCmd(ENABLE);		//设置WKUP用于唤醒
	PWR_EnterSTANDBYMode();			//进入待机模式
}

//低功耗，没用的引脚设置为模拟输入
void set_all_gpio_an_mode(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
}

//将没有使用的GPIO设置为模拟输入
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
 * 让CPU进入STOP
 * 注意从STOP模式回来要重新配置时钟 
 */
void enter_stop_mode(void)
{
	debug("所有GPIO全部设置为模拟输入");
	set_all_gpio_an_mode();
	debug("CPU正在进入STOP模式\r\n");
	stop_flag = 1;
	PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI); //进入停止模式，内核停止，保持外设工作。
}


/*
 * CPU从STOP模式回来，该函数仅能被唤醒CPU的中断函数调用
*/
void leave_stop_mode(void)
{
	if (stop_flag == 1) {
		stop_flag = 0;
		SystemInit(); //重新配置时钟
		debug("CPU已经被唤醒\r\n");
	}
}


#include "usb_bsp.h"
#include "usbd_conf.h"
#include "config.h"



/**
* @brief  USB_OTG_BSP_Init
*         USB OTG 底层IO初始化
* @param  pdev:USB OTG内核结构体指针
* @retval None
*/
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{  
	GPIO_InitTypeDef 						GPIO_InitStructure; 
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 		/* 使能GPIOA时钟 */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE); 		/*使能USB OTG时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	GPIO_InitStructure.GPIO_Pin 			= GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType 			= GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd 			= GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);  

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ; 	/* PA11,AF10(USB) */
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;	/* PA12,AF10(USB) */

	
}

/**
* @brief  USB OTG 中断设置,开启USB FS中断
*         使能USB的全局中断
* @param  None
* @retval None
*/
void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{
	NVIC_InitTypeDef NVIC_InitStructure; 
  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel 							= OTG_FS_IRQn;  
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 		= 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 				= 3;
	NVIC_InitStructure.NVIC_IRQChannelCmd 						= ENABLE;
	NVIC_Init(&NVIC_InitStructure);  
}
/**
* @brief  USB_OTG_BSP_uDelay
*         This function provides delay time in micro sec
* @param  usec : Value of delay required in micro sec
* @retval None
*/
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
	uint32_t count = 0;
	const uint32_t utime = (120 * usec);
	
	do {
		if ( ++count > utime )
		return ;
	} while (1);
}

static void rt_hw_us_delay(int us)
{
	rt_uint32_t delta;
	
	/* 获得延时经过的tick数 */
	us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
	/* 获得当前时间 */
	delta = SysTick->VAL;
	/* 循环获得当前时间，直到达到指定的时间后退出循环 */
	while (delta - SysTick->VAL< us);	
}


/**
* @brief   USB_OTG_BSP_mDelay
*          This function provides delay time in milli sec
* @param  msec : Value of delay required in milli sec
* @retval None
*/
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
//	USB_OTG_BSP_uDelay(msec * 1000); 		/* 阻塞 */
	rt_hw_us_delay(msec * 1000);							/* 非阻塞 */
}



/*****************************END OF FILE*************************/

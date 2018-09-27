#include "usb_bsp.h"
#include "usbd_conf.h"
#include "config.h"



/**
* @brief  USB_OTG_BSP_Init
*         USB OTG �ײ�IO��ʼ��
* @param  pdev:USB OTG�ں˽ṹ��ָ��
* @retval None
*/
void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{  
	GPIO_InitTypeDef 						GPIO_InitStructure; 
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 		/* ʹ��GPIOAʱ�� */
	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE); 		/*ʹ��USB OTGʱ�� */
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
* @brief  USB OTG �ж�����,����USB FS�ж�
*         ʹ��USB��ȫ���ж�
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
	
	/* �����ʱ������tick�� */
	us = us * (SysTick->LOAD/(1000000/RT_TICK_PER_SECOND));
	/* ��õ�ǰʱ�� */
	delta = SysTick->VAL;
	/* ѭ����õ�ǰʱ�䣬ֱ���ﵽָ����ʱ����˳�ѭ�� */
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
//	USB_OTG_BSP_uDelay(msec * 1000); 		/* ���� */
	rt_hw_us_delay(msec * 1000);							/* ������ */
}



/*****************************END OF FILE*************************/

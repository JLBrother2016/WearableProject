/*
*********************************************************************************************************
*
*	模块名称 : USB中断服务程序
*	文件名称 : usb_it.c
*	版    本 : V1.0
*	说    明 : 本文件存放USB中断服务程序。只需将该文件加入工程即可，无需再到 stm32f4xx_it.c 中添加这些ISR程序
*
*
*********************************************************************************************************
*/

#include "usb_core.h"
#include "usbd_core.h"
#include "usb_conf.h"
#include <rtthread.h>

extern USB_OTG_CORE_HANDLE  USB_OTG_dev;

/* Private function prototypes -----------------------------------------------*/
extern uint32_t USBD_OTG_ISR_Handler(USB_OTG_CORE_HANDLE *pdev);


void OTG_FS_IRQHandler(void)
{
	rt_interrupt_enter();
	USBD_OTG_ISR_Handler (&USB_OTG_dev);
	rt_interrupt_leave();
}



/************************************** (END OF FILE) *********************************/




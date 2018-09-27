/*
*********************************************************************************************************
*
*	ģ������ : USB�жϷ������
*	�ļ����� : usb_it.c
*	��    �� : V1.0
*	˵    �� : ���ļ����USB�жϷ������ֻ�轫���ļ����빤�̼��ɣ������ٵ� stm32f4xx_it.c �������ЩISR����
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




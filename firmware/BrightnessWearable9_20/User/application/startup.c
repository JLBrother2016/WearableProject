/*
 * File      : startup.c
 * This file is part of RT-Thread RTOS
 * COPYRIGHT (C) 2006, RT-Thread Develop Team
 *
 * The license and distribution terms for this file may be
 * found in the file LICENSE in this distribution or at
 * http://openlab.rt-thread.com/license/LICENSE
 *
 * Change Logs:
 * Date           Author       Notes
 * 2006-08-31     Bernard      first implementation
 * 2011-06-05     Bernard      modify for STM32F107 version
 */

#include <rthw.h>
#include <rtthread.h>

#include "stm32f4xx.h"
#include "board.h"
#include "ff.h"
#include "rtc.h"
#include "easy_rtthread.h"

/**
 * @addtogroup STM32
 */

/*@{*/

extern int  rt_application_init(void);
#ifdef RT_USING_FINSH
extern void finsh_system_init(void);
extern void finsh_set_device(const char* device);
#endif

#ifdef __CC_ARM
extern int Image$$RW_IRAM1$$ZI$$Limit;
#define STM32_SRAM_BEGIN    (&Image$$RW_IRAM1$$ZI$$Limit)
#elif __ICCARM__
#pragma section="HEAP"
#define STM32_SRAM_BEGIN    (__segment_end("HEAP"))
#else
extern int __bss_end;
#define STM32_SRAM_BEGIN    (&__bss_end)
#endif

/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{
    rt_kprintf("\n\r Wrong parameter value detected on\r\n");
    rt_kprintf("       file  %s\r\n", file);
    rt_kprintf("       line  %d\r\n", line);

    while (1);
}

/**
 * This function will startup RT-Thread RTOS.
 */
void rtthread_startup(void)
{
    /* init board */
	/* 板级初始化：包括中断向量、系统时钟、串口、SPI接口、GPIO、控制台等初始化 */
    rt_hw_board_init();

    /* show version */
	/* 显示RT-Thread Logo 以及版本信息 */
    rt_show_version();

    /* init timer system */
	/* 系统定时器初始化 */
    rt_system_timer_init();

	/* 系统堆空间内存初始化 */
    rt_system_heap_init((void*)STM32_SRAM_BEGIN, (void*)STM32_SRAM_END);
	
	RTC_init(); //RTC时钟初始化
	
	mount(); //挂载文件系统

//    f_mount(FS_SD, NULL);


	
    /* init scheduler system */
	/* 系统调度器初始化 */
    rt_system_scheduler_init();

    /* init application */
	/* 应用程序初始化，创建自己的应用线程 */
    rt_application_init();

#ifdef RT_USING_FINSH
    /* init finsh */
	/* Finsh 系统初始化 */
    finsh_system_init();
	/* 将Finsh重定向到UART */
    finsh_set_device( FINSH_DEVICE_NAME );
#endif

    /* init timer thread */
	/* 创建定时器线程 */
    rt_system_timer_thread_init();	/* 使用软件定时器时才需要软件定时器线程扫描软件定时器链表 */

    /* init idle thread */
	/* 创建系统空闲线程 */
    rt_thread_idle_init();
	
    /* start scheduler */
	/* 启动系统调度器 */
    rt_system_scheduler_start();

    /* never reach here */
    return;
}

/* 启动文件中的__main 最终会调用main()函数 */
int main(void)
{
    /* disable interrupt first */
	/* 关闭中断 */
    rt_hw_interrupt_disable();

    /* startup RT-Thread RTOS */
    rtthread_startup();

    return 0;
}

/*@}*/

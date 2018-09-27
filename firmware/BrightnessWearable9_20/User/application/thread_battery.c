#include "thread_battery.h"
#include "key.h"
#include "rtc.h"
#include "config.h"
#include "battery.h"
#include "led.h"
#include "cpuusage.h"


extern struct float_t battery;
extern struct float_t cpu;
extern volatile recflg_t flg;
extern uint8_t 		dsp_stop_flag;

static void led_display(struct led *dis_led);

void rt_battery_thread_entry(void *parameter)
{
	uint16_t battery_val = 0;
	char buf[64];
	
	struct led dev_led, *led;
	
	struct key pg_stat, *pg_detect;
	struct key chg_stat, *chg_detect;
	pg_detect = &pg_stat;
	chg_detect = &chg_stat;
	key_register(pg_detect, GPIOC, 1);
	key_register(chg_detect, GPIOA, 0); 
	
	led = &dev_led;
	led_register(led, GPIOC, 7);
	led->off(led);
	
//	RTC_set_wakeup_1s();			//配置WAKE UP中断,1秒钟中断一次
//	watchdog_init_sec(3);			/* 看门狗初始化, 需要3秒钟喂一次狗 */
	SystemCoreClockUpdate(); 		//更新SystemCoreClock变量， 每次改变系统时钟的时候都要调用这个函数
	
	
	while(1) {
		battery_val = get_battery_adc_val(ADC_Channel_9);
		battery.floatval = (double)(battery_val * 3.3 * 2 / 4096);
		battery.major = (uint16_t)battery.floatval;
		battery.minor = (uint16_t)(battery.floatval * 100) % 100;
//		watchdog_feed();
//		led->toggle(led);
		cpu_usage_get(&cpu.major, &cpu.minor);
//		rt_sprintf(buf, "%d.%d%%", cpu.major, cpu.minor);
//       debug("系统时钟: %dMHz. CPU使用率: %s\r\n", SystemCoreClock/1000000, buf);
		
		debug("当前电压为 %d.%dv\r\n", battery.major, battery.minor);
		if (pg_detect->is_key_down(pg_detect) && chg_detect->is_key_down(chg_detect)) {
//			debug("正在充电...\r\n");
			led->on(led);
		} else {
//			debug("未充电...\r\n");
			led->off(led);
		}
		
		if (battery.floatval < 3.5) {
//			debug("电量低，请及时充电!\r\n");
		} else {
			
		}

        //LED灯用于提示
        if (flg.mpu == 1 && flg.audio == 1 && flg.sensors == 1){
            led_display(led);
        } else if(flg.mpu == 0 && flg.audio == 0 
          && flg.sensors == 0 && dsp_stop_flag == 1){
            led->on(led);
        }
		
        
		sleep(10);
	}	
}

static void led_display(struct led *dis_led)
{
    dis_led->on(dis_led);
    delay_10ms(12);
    dis_led->off(dis_led);
    delay_10ms(12);
    dis_led->on(dis_led);
    delay_10ms(12);
    dis_led->off(dis_led);
}




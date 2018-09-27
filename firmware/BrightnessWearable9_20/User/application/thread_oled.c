#include "thread_oled.h"
#include "rtc.h"
#include "mpu6050.h"
#include "si7021.h"
#include "bh1750.h"
#include "bmp180.h"
#include "config.h"

extern rt_event_t oled_on;
extern mutex_flag_t screen_on;
extern struct float_t battery;

void display_mpu6050(struct device_oled *oled)
{
	int32_t val;
	/* MPU数据 */
	val = mpu6050.acce_last.x;
	if (val < 0) {
		oled->display_string(0, 16, "-", SIZE_16, NORMAL);
		val = -val;
	}
	else 
		oled->display_string(0, 16, "+", SIZE_16, NORMAL);
	oled->display_number( 8, 16, val, 4, SIZE_16);
	
	val = mpu6050.acce_last.y;
	if (val < 0) {
		oled->display_string(40, 16, "-", SIZE_16, NORMAL);
		val = -val;
	}
	else
		oled->display_string(40, 16, "+", SIZE_16, NORMAL);
	oled->display_number(48, 16, val, 4, SIZE_16);
	
	val = mpu6050.acce_last.z;
	if (val < 0) {
		oled->display_string(80, 16, "-", SIZE_16, NORMAL);
		val = -val;
	}
	else
		oled->display_string(80, 16, "+", SIZE_16, NORMAL);
	oled->display_number(88, 16, val, 4, SIZE_16);
	
	val = mpu6050.gyro_last.x;
	if (val < 0) {
		oled->display_string(0, 32, "-", SIZE_16, NORMAL);
		val = -val;
	}
	else
		oled->display_string(0, 32, "+", SIZE_16, NORMAL);
	oled->display_number( 8, 32, val, 4, SIZE_16);
	
	val = mpu6050.gyro_last.y;
	if (val < 0) {
		oled->display_string(40, 32, "-", SIZE_16, NORMAL);
		val = -val;
	}
	else
		oled->display_string(40, 32, "+", SIZE_16, NORMAL);
	oled->display_number(48, 32, val, 4, SIZE_16);
	
	val = mpu6050.gyro_last.z;
	if (val < 0) {
		oled->display_string(80, 32, "-", SIZE_16, NORMAL);
		val = -val;
	}
	else
		oled->display_string(80, 32, "+", SIZE_16, NORMAL);
	oled->display_number(88, 32, val, 4, SIZE_16);
}

void display_si7021(struct device_oled *oled)
{
	/* 温湿度 */
	oled->display_number( 0, 48, si7021.humidity/100, 2, SIZE_16);
	oled->display_string(16, 48, "%", SIZE_16, NORMAL);
	oled->display_number(26, 48, si7021.temperature/100, 2, SIZE_16);
	oled->display_string(42, 48, ".", SIZE_16, NORMAL);
	oled->display_number(48, 48, si7021.temperature/10%100, 1, SIZE_16);
}

void display_bmp180(struct device_oled *oled)
{
	/* 气压温度 */
	oled->display_number(65, 48, bmp180.pressure, 5, SIZE_16);
	oled->display_number(110, 48, bmp180.temperature/10, 2, SIZE_16);
}

void display_bh1750(struct device_oled *oled)
{
	/* 光线传感器 */
	oled->display_number(80, 0, (uint32_t)bh1750.lx, 5, SIZE_16);
}

void display_rtc(struct device_oled *oled)
{
	/* 时间 */
	rtc_t rtc;
	get_rtc_info(&rtc);
	oled->display_number(0, 0, rtc.hour, 2, SIZE_16);
	oled->display_string(16, 0, ":", SIZE_16, NORMAL);
	oled->display_number(24, 0, rtc.min, 2, SIZE_16);
	oled->display_string(40, 0, ":", SIZE_16, NORMAL);
	oled->display_number(48, 0, rtc.sec, 2, SIZE_16);
}

static void display_battery(struct device_oled *oled)
{
    oled->display_number(80, 0, battery.major, 1, SIZE_16);
    oled->display_string(88, 0, ".", SIZE_16, NORMAL);
    oled->display_number(96, 0, battery.minor, 2, SIZE_16);
    oled->display_string(112, 0, "V", SIZE_16, NORMAL);
}

void rt_oled_thread_entry(void *parameter)
{
	struct device_oled *oled;
	uint8_t countdown = 25;
	rt_uint32_t revent;
	
	oled = &device_oled;
	rt_device_oled_register(oled);
	
	oled->init(oled);
	oled->turn_on();
	
	while(1)
	{
		if (countdown == 0) {
			oled->turn_off();		//息屏
			
			mutex_lock(screen_on.mutex);
			screen_on.flag = 0;
			mutex_unlock(screen_on.mutex);
			
			event_recv(oled_on, EVENT_OLED_ON, RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, &revent); //线程在此休眠等待事件。
			if (revent & EVENT_OLED_ON) {
				countdown = 25;
			}
			
			mutex_lock(screen_on.mutex);
			screen_on.flag = 1;
			mutex_unlock(screen_on.mutex);
			
			oled->turn_on();		//亮屏
		}

		display_battery(oled);
		display_mpu6050(oled);
		display_si7021(oled);
		display_bmp180(oled);
		display_rtc(oled);

		/* 刷新屏幕 */
		oled->refresh_gram();
		delay_10ms(20);	
		countdown--;
	}	
}

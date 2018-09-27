#include <board.h>
#include <rtthread.h>

#ifdef RT_USING_GDB
#include <gdb_stub.h>
#endif

#include "config.h"
#include "thread_key.h"
#include "thread_mpu6050.h"
#include "thread_oled.h"
#include "thread_battery.h"
#include "usb_bsp_msc.h"
#include "thread_sensors.h"
#include "lowpower.h"
#include "watchdog.h"
#include "cpuusage.h"
#include "thread_file.h"
#include "thread_audio.h"
#include "thread_dsp.h"


FATFS 	fs;			//文件系统

struct mpu_buffer mpubuffer;
struct sensors_buffer sensorsbuf;
struct dsp_buffer dspdatabuf;
struct name_buffer old_name_wav;
struct fft_buffer in_dsp_databuf;

//文件操作事件
rt_event_t fopevent;

//文件操作完成事件
rt_event_t fopcmpl;

//打开oled的时间
rt_event_t oled_on;

//音频特征处理事件
rt_event_t audio_feature;


uint8_t screen_on_flag  = 1;				/* 屏幕开关状态标识 */
mutex_flag_t screen_on;
mutex_flag_t stop_key_dsp;
uint8_t stop_flag;

/* 用互斥量保护debug, err等调试信息 */
rt_mutex_t log_mutex;
struct float_t cpu = {0, 0, 0};				/* CPU使用率 */
struct float_t battery = {0, 0, 0};			/* 电池电压 */
volatile recflg_t flg;

rt_thread_t tid_key      = RT_NULL;
rt_thread_t tid_mpu      = RT_NULL;
rt_thread_t tid_sensor   = RT_NULL;
rt_thread_t tid_battery  = RT_NULL;
rt_thread_t tid_oled     = RT_NULL;
rt_thread_t tid_wm8978   = RT_NULL;
rt_thread_t tid_file     = RT_NULL;
rt_thread_t tid_dsp      = RT_NULL;




/* 线程初始化 */
int rt_application_init()
{
	struct thread_opt opt;
	
	opt.stack_size = 10240;
	opt.parameter = RT_NULL;
	opt.tick = 10;
	
	flg.sensors = 0;
	flg.mpu = 0;
	flg.audio = 0;
	flg.mutex = mutex_create("recording status");
	log_mutex = mutex_create("log");
	mpubuffer.mutex = mutex_create("mpu_buffer");
	sensorsbuf.mutex = mutex_create("sensorsbuf");
	screen_on.mutex = mutex_create("screen on");
	
	screen_on.flag = 1;
	old_name_wav.using_flag = 0;
	
	fopevent = event_create("fopevent");
	fopcmpl  = event_create("fopcmpl");
	audio_feature  = event_create("audio_feature");
	oled_on  = event_create("oledon");
	
	set_nouse_gpio_an_mode();
	
	thread_create("key_thread", 	rt_key_thread_entry, 		2, RT_NULL, &tid_key);
	thread_create("mpu_thread", 	rt_mpu_thread_entry, 		4, &opt,    &tid_mpu);
	thread_create("sensors_thread", rt_sensors_thread_entry, 	3, RT_NULL, &tid_sensor);
	thread_create("battery_thread", rt_battery_thread_entry, 	7, RT_NULL, &tid_battery);
	thread_create("oled_thread", 	rt_oled_thread_entry, 		6, RT_NULL, &tid_oled);  //OLED 是非常占用CPU资源的
	thread_create("wm8978_thread",  rt_wm8978_thread_entry, 	4, RT_NULL, &tid_wm8978);
	thread_create("file_thread", 	rt_file_thread_entry, 		4, &opt,    &tid_file);
	thread_create("dsp_thread", 	rt_dsp_thread_entry, 		8, RT_NULL, &tid_dsp);

	
	cpu_usage_init();
	
    return 0;   
}


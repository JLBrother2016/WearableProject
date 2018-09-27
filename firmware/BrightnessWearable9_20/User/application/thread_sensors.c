#include "thread_sensors.h"
#include "si7021.h"
#include "bh1750.h"
#include "bmp180.h"
#include "config.h"
#include "i2c.h"

#ifdef THREAD
#undef THREAD
#endif
#define THREAD 		"[Thread_Snesor]"

extern volatile recflg_t flg;
extern rt_event_t fopevent;
extern rt_event_t fopcmpl;
extern struct sensors_buffer sensorsbuf;

static void sensors_start_recording(void);
static void sensors_recording(bh1750_t *bh, bmp180_t *bmp, si7021_t *si);
static void sensors_stop_recording(void);

/* 温湿度、光线、气压传感器线程 */
void rt_sensors_thread_entry(void *parameter)
{
	i2c_bus_t 		i2c2;
	uint8_t 		status = 0;
	rt_uint32_t 	revent;
	rt_err_t		res;
	
	bh1750_t *bh = &bh1750;
	bmp180_t *bmp = &bmp180;
	si7021_t *si = &si7021;
	
	
	i2c_bus_register(&i2c2, GPIOB, 10, 11);
	bh1750_register(bh, &i2c2);
	bmp180_register(bmp, &i2c2);
	si7021_register(si, &i2c2);
	bmp->check_device(bmp);
	bh->check_device(bh);
	si->check_device(si);
	
	bmp->init(bmp);
	bh->init(bh);
	si->init(si);
	
	while(1) {
		
		if (flg.sensors == 1) {
			if (status == 0) {			//flag 从0到1  START动作
				status = 1;
				
				sensors_start_recording();
			}
			
			sensors_recording(bh, bmp, si);
		}
		
		else {  //flg.sensors != 1
			if (status == 1) {			//flag从1到0 STOP动作
				status = 0;
				sensors_stop_recording();
			}
		}
		
		res = event_recv_timeout(fopcmpl, SENSORS_CLOSE_OPEN, 
								RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
								100,
								&revent);
		if ((revent & SENSORS_CLOSE_OPEN) && (res == RT_EOK)) {
			debug(THREAD "SENSORS_CLOSE_OPEN事件到，写入新文件！\r\n");
			sensors_stop_recording();
			sensors_start_recording();
		}
	}
}

static void sensors_start_recording(void)
{
	rt_uint32_t 	revent;
	debug(THREAD "发送创建SENSOR文件事件~\r\n");
	event_send(fopevent, SENSORS_START);
	
	event_recv(fopcmpl, SENSORS_START_CMPL, 
				RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 
				&revent);
	
	if (revent & SENSORS_START_CMPL) {
		debug(THREAD "接收到SENSORS文件创建完成事件!\r\n");
	}
}

static void sensors_recording(bh1750_t *bh, bmp180_t *bmp, si7021_t *si)
{
	rt_uint32_t 	revent;
	//记录数据
	si->read_data(si);
	bh->get_lx(bh);	
	
	mutex_lock(sensorsbuf.mutex);
	bmp->read_data(bmp);
	rt_memset(sensorsbuf.buffer, 0, 64);
	rt_snprintf(sensorsbuf.buffer, 64, "%5d, %5d, %5d, %5d, %5d\n", 
		(uint32_t)bh->lx, bmp->pressure, bmp->temperature, si->humidity, si->temperature);
	mutex_unlock(sensorsbuf.mutex);
	
//			debug(THREAD "发送SENSORS写文件事件~\r\n");
	event_send(fopevent, SENSORS_EVENT);
	event_recv(fopcmpl, SENSORS_EVENT_CMPL, 
					RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 
					&revent);
	if (revent & SENSORS_EVENT_CMPL) {
//				debug(THREAD "接收到SENSORS文件写入完成事件!\r\n");
	}
}

static void sensors_stop_recording(void)
{
	rt_uint32_t 	revent;
	debug(THREAD "发送关闭SENSORS文件事件\r\n");
	event_send(fopevent, SENSORS_STOP);
	event_recv(fopcmpl, SENSORS_STOP_CMPL, 
			   RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 
			   &revent);
	if (revent & SENSORS_STOP_CMPL) {
		debug(THREAD "接收到SENSORS文件关闭完成事件!\r\n");
	}
	
	//进入待机模式
}


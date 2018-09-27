#include "thread_audio.h"
#include "audio.h"
#include "config.h"
#include "i2s.h"

#ifdef THREAD
#undef THREAD
#endif
#define THREAD 		"[Thread_Audio]"

extern volatile recflg_t flg;
extern rt_event_t fopevent;
extern rt_event_t fopcmpl;

static void audio_start_recording(wm8978_t *wm);
static void audio_recording(void);
static void audio_stop_recording(void);

void rt_wm8978_thread_entry(void *parameter)
{
	i2c_bus_t  i2c3;
	uint8_t status = 0;
	rt_err_t		res;
	rt_uint32_t 	revent;

	wm8978_t *wm = &wm8978;
	
	i2c_bus_register(&i2c3, GPIOB, 6, 7);					/* 注册I2C, PB6, PB7 */
	i2s_gpio_config();										/* I2S GPIO 配置 */
	wm8978_register(wm, &i2c3);								/* 注册WM8978 */
	wm->check_device(wm);									/* 检查I2C3总线上是否挂有WM8978 */
	wm->init(wm);											/* WM8978初始化 */
	wm->ioctl(wm, WM8978_CMD_SET_RECORDER_MODE, NULL);		/* 设置WM8978为录音模式 */
				
	while (1) {

		if (flg.audio == 1) {
			if (status == 0) {//flag 从0到1  START动作
				status = 1;
				audio_start_recording(wm);
			}
			
			//录音中
			
		}
		else {
			if (status == 1) {//flag从1到0 STOP动作
				status = 0;
				
				audio_stop_recording();
			}
		}
		
		res = event_recv_timeout(fopcmpl, AUDIO_CLOSE_OPEN, 
								RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
								5,
								&revent);
		if ((revent & AUDIO_CLOSE_OPEN) && (res == RT_EOK)) {
			debug(THREAD "AUDIO_CLOSE_OPEN事件到，音频数据写入新文件！\r\n");
			audio_stop_recording();
			audio_start_recording(wm);
		}
	}
}

static void audio_start_recording(wm8978_t *wm)
{
	rt_uint32_t 	revent;
	audio_set_record_mode(wm);
	
	debug(THREAD "发送创建AUDIO事件~\r\n");
	event_send(fopevent, AUDIO_START);
	
	event_recv(fopcmpl, AUDIO_START_CMPL, 
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
				&revent);
	if (revent & AUDIO_START_CMPL) {
		debug(THREAD "接收到AUDIO文件创建完成事件!\r\n");
	}
}

static void audio_recording(void)
{

}

static void audio_stop_recording(void)
{
	rt_uint32_t 	revent;
	
	recorder_stop();						/* 停止录音 */
	debug(THREAD "发送关闭AUDIO事件~\r\n");
	event_send(fopevent, AUDIO_STOP);
	
	event_recv(fopcmpl, AUDIO_STOP_CMPL, 
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
				&revent);
	if (revent & AUDIO_STOP_CMPL) {
		debug(THREAD "接收到AUDIO文件关闭完成事件!\r\n");
	}
}

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
	
	i2c_bus_register(&i2c3, GPIOB, 6, 7);					/* ע��I2C, PB6, PB7 */
	i2s_gpio_config();										/* I2S GPIO ���� */
	wm8978_register(wm, &i2c3);								/* ע��WM8978 */
	wm->check_device(wm);									/* ���I2C3�������Ƿ����WM8978 */
	wm->init(wm);											/* WM8978��ʼ�� */
	wm->ioctl(wm, WM8978_CMD_SET_RECORDER_MODE, NULL);		/* ����WM8978Ϊ¼��ģʽ */
				
	while (1) {

		if (flg.audio == 1) {
			if (status == 0) {//flag ��0��1  START����
				status = 1;
				audio_start_recording(wm);
			}
			
			//¼����
			
		}
		else {
			if (status == 1) {//flag��1��0 STOP����
				status = 0;
				
				audio_stop_recording();
			}
		}
		
		res = event_recv_timeout(fopcmpl, AUDIO_CLOSE_OPEN, 
								RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
								5,
								&revent);
		if ((revent & AUDIO_CLOSE_OPEN) && (res == RT_EOK)) {
			debug(THREAD "AUDIO_CLOSE_OPEN�¼�������Ƶ����д�����ļ���\r\n");
			audio_stop_recording();
			audio_start_recording(wm);
		}
	}
}

static void audio_start_recording(wm8978_t *wm)
{
	rt_uint32_t 	revent;
	audio_set_record_mode(wm);
	
	debug(THREAD "���ʹ���AUDIO�¼�~\r\n");
	event_send(fopevent, AUDIO_START);
	
	event_recv(fopcmpl, AUDIO_START_CMPL, 
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
				&revent);
	if (revent & AUDIO_START_CMPL) {
		debug(THREAD "���յ�AUDIO�ļ���������¼�!\r\n");
	}
}

static void audio_recording(void)
{

}

static void audio_stop_recording(void)
{
	rt_uint32_t 	revent;
	
	recorder_stop();						/* ֹͣ¼�� */
	debug(THREAD "���͹ر�AUDIO�¼�~\r\n");
	event_send(fopevent, AUDIO_STOP);
	
	event_recv(fopcmpl, AUDIO_STOP_CMPL, 
				RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, 
				&revent);
	if (revent & AUDIO_STOP_CMPL) {
		debug(THREAD "���յ�AUDIO�ļ��ر�����¼�!\r\n");
	}
}

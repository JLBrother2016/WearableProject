#include "thread_key.h"
#include "key.h"
#include "config.h"
#include "usb_bsp_msc.h"		/* usb�ײ����� */

#ifdef THREAD
#undef THREAD
#endif
#define THREAD 		"[Thread_Key]"

extern rt_event_t oled_on;
extern mutex_flag_t screen_on;
extern mutex_flag_t stop_key_dsp;

extern volatile recflg_t flg;
extern rt_event_t fopcmpl;
static rt_timer_t timer;

extern rt_thread_t tid_mpu;
extern rt_thread_t tid_sensor;
extern rt_thread_t tid_battery;
extern rt_thread_t tid_oled;
extern rt_thread_t tid_wm8978;
extern rt_thread_t tid_file;
extern rt_thread_t tid_dsp;



//��ʱ����ʱ����
static void timeout(void *parameter)
{
	if (flg.audio == 1) {
		rt_kprintf("[Timer] ����AUDIO_CLOSE_OPEN�¼�\r\n");
		event_send(fopcmpl, AUDIO_CLOSE_OPEN);
	}
	
	if (flg.mpu == 1) {
		rt_kprintf("[Timer] ����MPU_CLOSE_OPEN�¼�\r\n");
		event_send(fopcmpl, MPU_CLOSE_OPEN);
	}
	
	if (flg.sensors == 1) {
		rt_kprintf("[Timer] ����SENSORS_CLOSE_OPEN�¼�\r\n");
		event_send(fopcmpl, SENSORS_CLOSE_OPEN);
	}
	
}

/* 
 * ��������߳� 
 * Key1 PC13  ����
 * Key2 PC0   ˯��
 */
void rt_key_thread_entry(void *parameter)
{
	rt_err_t res;
	struct key key_start, key_stop;
	uint8_t status;
	key_event_t key_event[2];					/* ��������״̬ */				
	key_t *key[2];								/* �����ṹ��ָ�����飬��Ҫɨ��� */	
	key[0] = &key_start;
	key[1] = &key_stop;
	key_register(key[0], GPIOC, 13);
	key_register(key[1], GPIOC, 0); 

    
	
	while(1) {
		keys_scan(key, key_event);	
		
        if (key_event[0] == LONG_PRESS){
            debug("ʹ��USB�������豸");
            
            set_PLL48CLK(7);
            SystemCoreClockUpdate(); 		//����SystemCoreClock������ ÿ�θı�ϵͳʱ�ӵ�ʱ��Ҫ�����������

            rt_thread_delete(tid_mpu);
            rt_thread_delete(tid_sensor);
            rt_thread_delete(tid_battery);
            rt_thread_delete(tid_oled);
            rt_thread_delete(tid_wm8978);
            rt_thread_delete(tid_file);
            rt_thread_delete(tid_dsp);

            usbd_OpenMassStorage();      //����USB�������豸

        }

        if (key_event[1] == LONG_PRESS){
            debug("�ر�USB�������豸");
            usbd_CloseMassStorage();	 //�ر�USB�������豸
        }

		if (key_event[0] == SINGLE_CLICK) {//��ʼ��¼
			debug(THREAD "��ʼ��������!\r\n");

			mutex_lock(screen_on.mutex);
			status = screen_on.flag;
			mutex_unlock(screen_on.mutex);
			
			if (status == 1) {
				mutex_lock(flg.mutex);
				flg.sensors = 1;
				flg.mpu = 1;
				flg.audio = 1;
				mutex_unlock(flg.mutex);
				
				timer = rt_timer_create("timer", timeout, RT_NULL, SET_TIMER, RT_TIMER_FLAG_PERIODIC);

				if (timer != RT_NULL) {
					rt_timer_start(timer);
					debug(THREAD "������ʱ���ɹ�\r\n");
				} else {
					debug(THREAD "������ʱ��ʧ��!\r\n");
				}
				stop_key_dsp.flag = 0;
			} else {
				event_send(oled_on, EVENT_OLED_ON);
			}
		}
		if (key_event[1] == SINGLE_CLICK) {//ֹͣ��¼
			debug(THREAD "ֹͣ��������!\r\n");			

			mutex_lock(screen_on.mutex);
			status = screen_on.flag;
			mutex_unlock(screen_on.mutex);
			
			if (status == 1) {
				event_send(oled_on, EVENT_OLED_ON);
				mutex_lock(flg.mutex);
				flg.sensors = 0;
				flg.mpu = 0;
				flg.audio = 0;
				mutex_unlock(flg.mutex);
				
				res = rt_timer_delete(timer);
				if (res != RT_EOK) {
					debug(THREAD "ɾ����ʱ��ʧ��!\r\n");
				} else {
					debug(THREAD "ɾ����ʱ���ɹ�!\r\n");
				}
				stop_key_dsp.flag = 1;
				
			} else {
				event_send(oled_on, EVENT_OLED_ON);
			}		
//			debug(THREAD "3������STOPģʽ\r\n");
//			mutex_lock(flg.mutex);
//			flg.oled = 0;
//			mutex_unlock(flg.mutex);
//			sleep(3);
//			debug(THREAD "���ڽ���STOPģʽ\r\n");
//			stop_flag = 1;
//			flg.oled = 0;
//			exti_init();
//			enter_stop_mode();
//			debug(THREAD "��STOPģʽ����\r\n");
//			flg.oled = 1;

		}
		rt_thread_delay(2);
	}
}


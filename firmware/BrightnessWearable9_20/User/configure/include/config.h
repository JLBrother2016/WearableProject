#ifndef _CONFIG_H_
#define _CONFIG_H_

/* SD ���������� */
#define SD_DETECT_PIN                    GPIO_Pin_3
#define SD_DETECT_GPIO_PORT              GPIOB
#define SD_DETECT_GPIO_CLK               RCC_AHB1Periph_GPIOB

#define MPU_BUFFER_SIZE 				 512
#define SENSOR_BUFFER_SIZE 				 64
#define FEATURE_BUFFER_SIZE 		 128
#define DSP_BUFFER_SIZE 				 512
#define I2S_DAM_BUFFER_SIZE 			 512	     //10K
#define AUDIO_SAMPLE_RATE				 8000        //��Ƶ������
#define WIN_LEN_SIZE					 256

#define FILE_NAME_LENGTH				 64

#define SET_TIMER			        	 60000       //���ö�ʱ���Զ��򿪹ر��ļ���10msһ��ϵͳ����

#define EVENT_OLED_ON 	(1 << 0)


//���ݲɼ��̷߳��͸��ļ������߳�
#define AUDIO_EVENT_0 		(1 << 0)
#define AUDIO_EVENT_1		(1 << 1)
#define MPU_EVENT 			(1 << 2)
#define SENSORS_EVENT 		(1 << 3)
#define AUDIO_START			(1 << 4)
#define MPU_START			(1 << 5)
#define SENSORS_START		(1 << 6)
#define AUDIO_STOP			(1 << 7)
#define MPU_STOP			(1 << 8)
#define SENSORS_STOP		(1 << 9)
#define DSP_START			(1 << 10)
#define DSP_STOP			(1 << 11)
#define DSP_READ_EVENT		(1 << 12)
#define DSP_WRITE_EVENT		(1 << 13)

//�ļ������̷߳��͸����ݲɼ��߳�
#define AUDIO_EVENT_0_CMPL 	(1 << 0)
#define AUDIO_EVENT_1_CMPL	(1 << 1)
#define MPU_EVENT_CMPL 		(1 << 2)
#define SENSORS_EVENT_CMPL 	(1 << 3)
#define AUDIO_START_CMPL	(1 << 4)
#define MPU_START_CMPL		(1 << 5)
#define SENSORS_START_CMPL	(1 << 6)
#define AUDIO_STOP_CMPL		(1 << 7)
#define MPU_STOP_CMPL		(1 << 8)
#define SENSORS_STOP_CMPL	(1 << 9)
//��ʱ���̷߳��͸����ݲɼ��̵߳��ź�
#define AUDIO_CLOSE_OPEN	(1 << 10)
#define MPU_CLOSE_OPEN		(1 << 11)
#define SENSORS_CLOSE_OPEN	(1 << 12)

//DSP������һ�������ļ����ź�
#define DSP_START_CMPL		(1 << 0)
#define DSP_STOP_CMPL		(1 << 1)
#define DSP_READ_EVENT_CMPL	(1 << 2)
#define DSP_WRITE_EVENT_CMPL (1 << 3)
#define DSP_READY_STOP		(1 << 4)
#define DSP_COMPUTE_AGAIN	(1 << 5)


#include <rtthread.h>
#include "easy_rtthread.h"
#include "ff.h"
#include "window.h"

/* �û���������debug, err�ȵ�����Ϣ */
extern rt_mutex_t log_mutex;

#ifdef __DEBUG
#define debug(...) do {	\
						mutex_lock(log_mutex);	 \
						rt_kprintf(__VA_ARGS__); \
						mutex_unlock(log_mutex); \
					} while(0)
#else
#define debug(...)
#endif
					
#ifdef __ERROR
#define err(...) do {	\
						mutex_lock(log_mutex);	 \
						rt_kprintf("Error! [%s: %d]", __FILE__, __LINE__); \
						rt_kprintf(__VA_ARGS__); \
						mutex_unlock(log_mutex); \
					} while(0)
#else
#define err(...)
#endif			

#define NULL     		0
#define malloc 			rt_malloc
#define free 			rt_free
			
/* ˯�ߺ��� */		
#define delay_ms(val)	rt_thread_delay(val/10)		//��λms	
#define delay_10ms 		rt_thread_delay				//��λ10ms
#define sleep(val)		rt_thread_delay(val*100) 	//��λ��
					
/* I2C �ȴ�ACK�ĳ�ʱʱ�� */
#define I2C_WAIT_ACK_TIMEOUT 	10000			
					
#define assert(EX)                                                            	\
if (!(EX)) {                                                                  	\
    volatile char dummy = 0;                                                 	\
    rt_kprintf("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__); 	\
    while (dummy == 0);                                                       	\
}

//MPU ���ݵ�buffer
struct mpu_buffer {
	rt_mutex_t 		mutex;
	char  				buffer[MPU_BUFFER_SIZE];
	uint32_t 			used;
	uint8_t 			full;
};

/* CPUʹ���� major.minor% */
struct float_t {
	uint8_t 	major;
	uint8_t 	minor;
	double 		floatval;
};

//SENSOR ���� ��buffer
struct sensors_buffer {
	rt_mutex_t mutex;
	char 	   buffer[SENSOR_BUFFER_SIZE];
};
//DSP ���� ��buffer
struct dsp_buffer {
	rt_mutex_t mutex;
	int16_t    buffer[DSP_BUFFER_SIZE]; 			//��wav��ʽ�������ļ���һ֡����
	char	   feature_buffer[FEATURE_BUFFER_SIZE];	//�������������ء������Ƶ�ʵ�����
};

//�������е�WAV��ʽ�������ļ���
struct name_buffer {
	rt_mutex_t mutex;
	char    name_buffer1[FILE_NAME_LENGTH];			
	char	name_buffer2[FILE_NAME_LENGTH];	
	uint8_t 			first;
	uint32_t 			second;
	uint8_t				using_flag;
};
//FFT���㷨��Ҫ��buffer
struct fft_buffer {
	rt_mutex_t mutex;
	float    energy_buffer_voice[WIN_LEN_SIZE];
	float 	 entropy_buffer_voice[WIN_LEN_SIZE];
	float 	 formant_buffer_voice[WIN_LEN_SIZE];
};
					
#endif

#ifndef _CONFIG_H_
#define _CONFIG_H_

/* SD 卡检测的引脚 */
#define SD_DETECT_PIN                    GPIO_Pin_3
#define SD_DETECT_GPIO_PORT              GPIOB
#define SD_DETECT_GPIO_CLK               RCC_AHB1Periph_GPIOB

#define MPU_BUFFER_SIZE 				 512
#define SENSOR_BUFFER_SIZE 				 64
#define FEATURE_BUFFER_SIZE 		 128
#define DSP_BUFFER_SIZE 				 512
#define I2S_DAM_BUFFER_SIZE 			 512	     //10K
#define AUDIO_SAMPLE_RATE				 8000        //音频采样率
#define WIN_LEN_SIZE					 256

#define FILE_NAME_LENGTH				 64

#define SET_TIMER			        	 60000       //设置定时器自动打开关闭文件，10ms一个系统节拍

#define EVENT_OLED_ON 	(1 << 0)


//数据采集线程发送给文件操作线程
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

//文件操作线程发送给数据采集线程
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
//定时器线程发送给数据采集线程的信号
#define AUDIO_CLOSE_OPEN	(1 << 10)
#define MPU_CLOSE_OPEN		(1 << 11)
#define SENSORS_CLOSE_OPEN	(1 << 12)

//DSP处理上一个语音文件的信号
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

/* 用互斥量保护debug, err等调试信息 */
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
			
/* 睡眠函数 */		
#define delay_ms(val)	rt_thread_delay(val/10)		//单位ms	
#define delay_10ms 		rt_thread_delay				//单位10ms
#define sleep(val)		rt_thread_delay(val*100) 	//单位秒
					
/* I2C 等待ACK的超时时间 */
#define I2C_WAIT_ACK_TIMEOUT 	10000			
					
#define assert(EX)                                                            	\
if (!(EX)) {                                                                  	\
    volatile char dummy = 0;                                                 	\
    rt_kprintf("(%s) assert failed at %s:%d \n", #EX, __FUNCTION__, __LINE__); 	\
    while (dummy == 0);                                                       	\
}

//MPU 数据单buffer
struct mpu_buffer {
	rt_mutex_t 		mutex;
	char  				buffer[MPU_BUFFER_SIZE];
	uint32_t 			used;
	uint8_t 			full;
};

/* CPU使用率 major.minor% */
struct float_t {
	uint8_t 	major;
	uint8_t 	minor;
	double 		floatval;
};

//SENSOR 数据 单buffer
struct sensors_buffer {
	rt_mutex_t mutex;
	char 	   buffer[SENSOR_BUFFER_SIZE];
};
//DSP 数据 单buffer
struct dsp_buffer {
	rt_mutex_t mutex;
	int16_t    buffer[DSP_BUFFER_SIZE]; 			//从wav格式的语音文件读一帧数据
	char	   feature_buffer[FEATURE_BUFFER_SIZE];	//保存能量、谱熵、共振峰频率的数据
};

//保存已有的WAV格式的语音文件名
struct name_buffer {
	rt_mutex_t mutex;
	char    name_buffer1[FILE_NAME_LENGTH];			
	char	name_buffer2[FILE_NAME_LENGTH];	
	uint8_t 			first;
	uint32_t 			second;
	uint8_t				using_flag;
};
//FFT等算法需要的buffer
struct fft_buffer {
	rt_mutex_t mutex;
	float    energy_buffer_voice[WIN_LEN_SIZE];
	float 	 entropy_buffer_voice[WIN_LEN_SIZE];
	float 	 formant_buffer_voice[WIN_LEN_SIZE];
};
					
#endif

#include "thread_mpu6050.h"
#include "config.h"
#include "i2c.h"
#include "mpu6050.h"

#ifdef THREAD
#undef THREAD
#endif
#define THREAD 		"[Thread_MPU6050]"

extern volatile recflg_t flg;
extern rt_event_t fopevent;
extern rt_event_t fopcmpl;
extern struct mpu_buffer mpubuffer;

static void mpu_start_recording(mpu6050_t *mpu);
static void mpu_stop_recording(mpu6050_t *mpu);
static void mpu_recording(mpu6050_t *mpu);

/* MPU6050�Ĳ���Ƶ��Ϊ100Hz */
void rt_mpu_thread_entry(void *parameter)
{
	i2c_bus_t 			i2c1;
	uint8_t 			status = 0;
	rt_err_t			res;	
	rt_uint32_t 		revent;
	
	mpu6050_t *mpu = &mpu6050;
	
	i2c_bus_register(&i2c1, GPIOB, 8, 9);
	mpu6050_register(mpu, &i2c1);
	
	mpu->init(mpu);
	
	while(1)
	{
		if (flg.mpu == 1) {
			if (status == 0) { 				//flag ��0��1  START����������MPU���ݲɼ��ļ�
				status = 1;
				mpu_start_recording(mpu);
			}
			mpu_recording(mpu);
		}
		
		else { //flg.mpu != 1
			if (status == 1) { 				//flag��1��0 STOP�������ر�MPU���ݲɼ��ļ�
				status = 0;
				mpu_stop_recording(mpu);
			}
		}
		
		res = event_recv_timeout(fopcmpl, MPU_CLOSE_OPEN, 
								RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR,
								1,
								&revent);
		if ((revent & MPU_CLOSE_OPEN) && (res == RT_EOK)) {
			debug(THREAD "MPU_CLOSE_OPEN�¼�����д�����ļ���\r\n");
			mpu_stop_recording(mpu);
			mpu_start_recording(mpu);
		}
	}
}

static void mpu_start_recording(mpu6050_t *mpu)
{
	rt_uint32_t 		revent;
			
	mpubuffer.used = 0;
	mpubuffer.full = 0;
	rt_memset(mpubuffer.buffer, 0, MPU_BUFFER_SIZE);
	
	debug(THREAD "���ʹ���MPU�ļ��¼�~\r\n");
	event_send(fopevent, MPU_START);
	
	event_recv(fopcmpl, MPU_START_CMPL, 
				RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 
				&revent);
	if (revent & MPU_START_CMPL) {
		debug(THREAD "���յ�MPU�ļ���������¼�!\r\n");
	}
}

static void mpu_stop_recording(mpu6050_t *mpu)
{
	rt_uint32_t 		revent;
	debug(THREAD "���͹ر�MPU�ļ��¼�~\r\n");
	event_send(fopevent, MPU_STOP);
	event_recv(fopcmpl, MPU_STOP_CMPL, 
			   RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 
			   &revent);
	
	if (revent & MPU_STOP_CMPL) {
		debug(THREAD "���յ�MPU�ļ��ر�����¼�!\r\n");
	}				
	//mpu6050�������ģʽ
}

static void mpu_recording(mpu6050_t *mpu)
{
	rt_uint32_t 		revent;
	char  				buffer[64];
	uint32_t 			size;
	
	mutex_lock(mpubuffer.mutex);
	//��¼����
	mpu->read_data(mpu);
	rt_memset(buffer, 0, 64);
	
	size = rt_snprintf(buffer, 64, "%4d, %4d, %4d, %4d, %4d, %4d\n", 
						mpu->acce_last.x, 
						mpu->acce_last.y, 
						mpu->acce_last.z, 
						mpu->gyro_last.x, 
						mpu->gyro_last.y, 
						mpu->gyro_last.z);
	
	rt_memcpy(mpubuffer.buffer+mpubuffer.used, buffer, size);
	mutex_unlock(mpubuffer.mutex);
	mpubuffer.used += size;
	if (mpubuffer.used + size > MPU_BUFFER_SIZE) { //��������
		mpubuffer.full = 1;

		event_send(fopevent, MPU_EVENT);		 //֪ͨ�ļ������߳�д����
		event_recv(fopcmpl, MPU_EVENT_CMPL, 
					RT_EVENT_FLAG_AND | RT_EVENT_FLAG_CLEAR, 
					&revent);
		if (revent & MPU_EVENT_CMPL) {
//					debug(THREAD "���յ�MPU�ļ�д������¼�!\r\n");
		}
	}
}


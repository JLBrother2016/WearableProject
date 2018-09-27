#ifndef _BSP_MPU6050_H_
#define _BSP_MPU6050_H_

#include <stm32f4xx.h>
#include <rtthread.h>
#include "i2c.h"

#ifdef __cplusplus
	extern "C" {
#endif

/* 
 * MPU6050��������
 */
#define MPU6050_CMD_SET_GYRO_FSR 			1
#define MPU6050_CMD_SET_ACCE_FSR			2
#define MPU6050_CMD_SET_LPF					3
#define MPU6050_CMD_SET_SAMPLE_RATE			4
#define MPU6050_CMD_SET_GYRO_OFFSET			5
		
typedef struct _tag_int16_xyz_t
{
	int16_t x;
	int16_t y;
	int16_t z;
} mpu6050_int16_xyz_t;

typedef struct _tag_int32_xyz_t
{
	int32_t x;
	int32_t y;
	int32_t z;
} mpu6050_int32_xyz_t;

typedef enum
{
	ACCEL_FSR_2g = 0,
	ACCEL_FSR_4g,
	ACCEL_FSR_8g,
	ACCEL_FSR_16g
} mpu6050_acc_fsr_t;

#define IS_ACCE_FSR_Type(type) 	((type) == (ACCEL_FSR_2g) || \
								 (type) == (ACCEL_FSR_4g) || \
								 (type) == (ACCEL_FSR_8g) || \
							     (type) == (ACCEL_FSR_16g))

typedef enum
{
	GYRO_FSR_250 = 0,
	GYRO_FSR_500,
	GYRO_FSR_1000,
	GYRO_FSR_2000
} mpu6050_gyro_fsr_t;

#define IS_GYRO_FSR_Type(type) 	((type) == (GYRO_FSR_250)  || \
								 (type) == (GYRO_FSR_500)  || \
								 (type) == (GYRO_FSR_1000) || \
							     (type) == (GYRO_FSR_2000))

typedef enum
{
	SAMPLE_RATE_125Hz = 0,
	SAMPLE_RATE_250Hz,
	SAMPLE_RATE_500Hz,
	SAMPLE_RATE_1000Hz
} mpu6050_sample_rate_t;

#define IS_MPU_SAMPLE_RATE_Type(type) 	((type) == (SAMPLE_RATE_125Hz)  || \
										 (type) == (SAMPLE_RATE_250Hz)  || \
										 (type) == (SAMPLE_RATE_500Hz)  || \
										 (type) == (SAMPLE_RATE_1000Hz))

typedef enum
{
	LPF_5Hz = 0,
	LPF_10Hz,
	LPF_20Hz,
	LPF_40Hz,
	LPF_90Hz,
	LPF_180Hz,
	LPF_250Hz
} mpu6050_lpf_t;

#define IS_MPU_LPF_Type(type) 			((type) == (LPF_5Hz)   || \
										 (type) == (LPF_10Hz)  || \
										 (type) == (LPF_20Hz)  || \
										 (type) == (LPF_40Hz)  || \
										 (type) == (LPF_90Hz)  || \
										 (type) == (LPF_180Hz) || \
										 (type) == (LPF_250Hz))


typedef struct mpu6050 mpu6050_t;
struct mpu6050 {
	i2c_bus_t 					*i2c;
//	rt_mutex_t 					mutex;
	int32_t (*check_device)(mpu6050_t *mpu);
	int32_t (*init)(mpu6050_t *mpu);
	int32_t (*read_data)(mpu6050_t *mpu);
	int32_t (*ioctl)(mpu6050_t *mpu, uint8_t cmd, int32_t val);
	int32_t (*write_reg)(mpu6050_t *mpu, uint8_t reg, uint8_t val);
	int32_t (*read_reg)(mpu6050_t *mpu, uint8_t reg, uint8_t *val);
	
	mpu6050_acc_fsr_t 					acce_fsr;			/* ���ٶ������� */
	mpu6050_gyro_fsr_t 					gyro_fsr;			/* ������������ */
	mpu6050_sample_rate_t 				sr;					/* ������ */
	mpu6050_lpf_t							lpf;				/* ��ͨ�˲�ֵ */
	
	mpu6050_int16_xyz_t 					acce_avg;			/* ƽ��ֵ�˲����accel value */
	mpu6050_int16_xyz_t 					acce_last;			/* �������������� */
	mpu6050_int16_xyz_t 					gyro_last;			/* �������������� */
	
	mpu6050_int16_xyz_t 					offset_acce;		/* ���ٶ���ƫ */
	mpu6050_int16_xyz_t 					offset_gyro;		/* ��������ƫ */
	
	uint8_t 								buffer[14];			/* ���ݻ��� */
	uint8_t 								gyro_cal_flag;		/* ������У׼ */
	uint8_t 								acce_cal_flag;		/* ���ٶ�У׼ */
	uint8_t 								init_flag;			/* ��ʼ���ɹ���־ */	
};


extern mpu6050_t mpu6050;
int32_t mpu6050_register(mpu6050_t *mpu, i2c_bus_t *i2c);

#ifdef __cplusplus
}
#endif

#endif

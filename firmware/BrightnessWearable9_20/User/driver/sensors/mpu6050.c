#include "mpu6050.h"
#include "config.h"

#undef 	DEVICE
#define DEVICE 					"[MPU6050]: "

/* global variable. */
mpu6050_t 	mpu6050;


#define 	SELF_TEST_X			0X0D	/* �Լ�Ĵ���X */
#define 	SELF_TEST_Y			0X0E	/* �Լ�Ĵ���Y */
#define 	SELF_TEST_Z			0X0F	/* �Լ�Ĵ���Z */
#define 	SELF_TEST_A			0X10	/* �Լ�Ĵ���A */
#define 	SMPLRT_DIV			0X19	/* ����Ƶ�ʷ�Ƶ�� */
#define 	CONFIG				0X1A	/* ���üĴ��� */
#define 	GYRO_CONFIG			0X1B	/* ���������üĴ��� */
#define 	ACCEL_CONFIG		0X1C	/* ���ٶȼ����üĴ��� */
#define 	MOT_THR				0X1F	/* �˶���ֵⷧ���üĴ��� */
#define 	FIFO_EN				0X23	/* FIFOʹ�ܼĴ��� */
#define 	I2C_MST_CTRL		0X24	/* I2C�������ƼĴ��� */
#define 	I2C_SLV0_ADDR		0X25	/* I2C�ӻ�0������ַ�Ĵ��� */
#define 	I2C_SLV0_REG		0X26	/* I2C�ӻ�0���ݵ�ַ�Ĵ��� */
#define 	I2C_SLV0_CTRL		0X27	/* I2C�ӻ�0���ƼĴ��� */
#define 	I2C_SLV1_ADDR		0X28	/* I2C�ӻ�1������ַ�Ĵ��� */
#define 	I2C_SLV1_REG		0X29	/* I2C�ӻ�1���ݵ�ַ�Ĵ��� */
#define 	I2C_SLV1_CTRL		0X2A	/* I2C�ӻ�1���ƼĴ��� */
#define 	I2C_SLV2_ADDR		0X2B	/* I2C�ӻ�2������ַ�Ĵ��� */
#define 	I2C_SLV2_REG		0X2C	/* I2C�ӻ�2���ݵ�ַ�Ĵ��� */
#define 	I2C_SLV2_CTRL		0X2D	/* I2C�ӻ�2���ƼĴ��� */
#define 	I2C_SLV3_ADDR		0X2E	/* I2C�ӻ�3������ַ�Ĵ��� */
#define 	I2C_SLV3_REG		0X2F	/* I2C�ӻ�3���ݵ�ַ�Ĵ��� */
#define 	I2C_SLV3_CTRL		0X30	/* I2C�ӻ�3���ƼĴ��� */
#define 	I2C_SLV4_ADDR		0X31	/* I2C�ӻ�4������ַ�Ĵ��� */
#define 	I2C_SLV4_REG		0X32	/* I2C�ӻ�4���ݵ�ַ�Ĵ��� */
#define 	I2C_SLV4_DO			0X33	/* I2C�ӻ�4д���ݼĴ��� */
#define 	I2C_SLV4_CTRL		0X34	/* I2C�ӻ�4���ƼĴ��� */
#define 	I2C_SLV4_DI			0X35	/* I2C�ӻ�4�����ݼĴ��� */
#define 	I2C_MST_STATUS		0X36	/* I2C����״̬�Ĵ��� */
#define 	INT_PIN_CFG			0X37	/* �ж�/��·���üĴ��� */
#define 	INT_ENABLE			0X38	/* �ж�ʹ�ܼĴ��� */
#define 	INT_STATUS			0X3A	/* �ж�״̬�Ĵ��� */
#define 	ACCEL_XOUT_H		0X3B	/* ���ٶ�ֵ,X���8λ�Ĵ��� */
#define 	ACCEL_XOUT_L		0X3C	/* ���ٶ�ֵ,X���8λ�Ĵ��� */
#define 	ACCEL_YOUT_H		0X3D	/* ���ٶ�ֵ,Y���8λ�Ĵ��� */
#define 	ACCEL_YOUT_L		0X3E	/* ���ٶ�ֵ,Y���8λ�Ĵ��� */
#define 	ACCEL_ZOUT_H		0X3F	/* ���ٶ�ֵ,Z���8λ�Ĵ��� */
#define 	ACCEL_ZOUT_L		0X40	/* ���ٶ�ֵ,Z���8λ�Ĵ��� */
#define 	TEMP_OUT_H			0X41	/* �¶�ֵ��8λ�Ĵ��� */
#define 	TEMP_OUT_L			0X42	/* �¶�ֵ��8λ�Ĵ��� */
#define 	GYRO_XOUT_H			0X43	/* ������ֵ,X���8λ�Ĵ��� */
#define 	GYRO_XOUT_L			0X44	/* ������ֵ,X���8λ�Ĵ��� */
#define 	GYRO_YOUT_H			0X45	/* ������ֵ,Y���8λ�Ĵ��� */
#define 	GYRO_YOUT_L			0X46	/* ������ֵ,Y���8λ�Ĵ��� */
#define 	GYRO_ZOUT_H			0X47	/* ������ֵ,Z���8λ�Ĵ��� */
#define 	GYRO_ZOUT_L			0X48	/* ������ֵ,Z���8λ�Ĵ��� */
#define 	I2C_SLV0_DO			0X63	/* I2C�ӻ�0���ݼĴ��� */
#define 	I2C_SLV1_DO			0X64	/* I2C�ӻ�1���ݼĴ��� */
#define 	I2C_SLV2_DO			0X65	/* I2C�ӻ�2���ݼĴ��� */
#define 	I2C_SLV3_DO			0X66	/* I2C�ӻ�3���ݼĴ��� */
#define 	I2C_MST_DELAY_CTRL	0X67	/* I2C������ʱ����Ĵ��� */
#define 	SIGPATH_PATH_RESET	0X68	/* �ź�ͨ����λ�Ĵ��� */
#define 	MOT_DETECT_CTRL		0X69	/* �˶������ƼĴ��� */
#define 	USER_CTRL			0X6A	/* �û����ƼĴ��� */
#define 	PWR_MGMT1			0X6B	/* ��Դ����Ĵ���1 */
#define 	PWR_MGMT2			0X6C	/* ��Դ����Ĵ���2 */
#define 	FIFO_COUNTH			0X72	/* FIFO�����Ĵ�����8λ */
#define 	FIFO_COUNTL			0X73	/* FIFO�����Ĵ�����8λ */
#define 	FIFO_R_W			0X74	/* FIFO��д�Ĵ��� */
#define 	WHO_AM_I			0X75	/* ����ID�Ĵ��� */

#define 	MPU6050_ADDR 		0x68  	/* ADO ��GND */


/* ����ΪGPIOģ��MPU_I2C, �ܽ��������� */
/*
  *  +-------------------------------------------------------------------------+
  *  |																		   |
  *	 |						SCL 	------------- 	PB8					       |
  *	 |						SDA		-------------	PB9					       |
  *  |																		   |
  *	 +-------------------------------------------------------------------------+
*/


int32_t mpu6050_init(mpu6050_t *mpu)
{
	/* �����Ǻͼ��ٶȼƾ�δУ׼ */
	uint8_t reg;
	mpu->init_flag = 0;							
	mpu->acce_cal_flag = 0;
	mpu->gyro_cal_flag = 0;
	
	assert(mpu);
	mpu->read_reg(mpu, WHO_AM_I, &reg);
	if (reg != MPU6050_ADDR) {
		debug(DEVICE "MPU6050 not found!\r\n");
		return -1;
	}
	
	debug(DEVICE "ʶ�� MPU6050...\r\n");
	debug(DEVICE "MPU6050 ������ַΪ 0x %x...\r\n", MPU6050_ADDR);
	mpu->write_reg(mpu, PWR_MGMT1, 0x80);							/* reset mpu6050 */	
	debug(DEVICE "���ڸ�λMPU6050... \r\n");
	delay_10ms(100);
	mpu->write_reg(mpu, PWR_MGMT1, 0x00);							/* wakeup mpu6050, select internal 8MHz oscillator as clock source */
	debug(DEVICE "���ڻ���MPU6050... \r\n");
	delay_10ms(100);
	mpu->ioctl(mpu, MPU6050_CMD_SET_GYRO_FSR, GYRO_FSR_2000);		/* config gyro full scale range to +-2000��/S */				
	debug(DEVICE "����������Ϊ�� + - 2000��/��. \r\n");
	mpu->ioctl(mpu, MPU6050_CMD_SET_ACCE_FSR, ACCEL_FSR_4g);		/* config accel full sacle range to +-4g */
	debug(DEVICE "���ٶȼ�����Ϊ�� + -4g. \r\n");
	mpu->ioctl(mpu, MPU6050_CMD_SET_SAMPLE_RATE, SAMPLE_RATE_125Hz);/* config sample rate to 50HZ, low pass filter to 125Hz */
	debug(DEVICE "������Ϊ��125Hz. \r\n");
	mpu->ioctl(mpu, MPU6050_CMD_SET_LPF, LPF_5Hz);
	debug(DEVICE "���ֵ�ͨ�˲�������Ϊ 5 Hz. \r\n");
	mpu->write_reg(mpu, INT_ENABLE, 0x00);							/* disable all interrupts */
	debug(DEVICE "�ر������ж�. \r\n");
	mpu->write_reg(mpu, USER_CTRL, 0x00);							/* disable fifo, disable MPU_I2C mast mode*/
	debug(DEVICE "�ر�FIFO�� �ر�MPU_I2C��ģʽ. \r\n");
	mpu->write_reg(mpu, FIFO_EN, 0x00);								/* disable all fifo */
	mpu->write_reg(mpu, INT_PIN_CFG, 0x80);							/* INT Pin active Low, push-pull, 50us pulse */	
	mpu->write_reg(mpu, PWR_MGMT1, 0x01);							/* select PLL with X axis gyroscope reference */
	debug(DEVICE "ѡ��������X����ΪPLL�ο�. \r\n");
	mpu->write_reg(mpu, PWR_MGMT2, 0x00);							/* disbale low power mode */
	debug(DEVICE "�رյ͹���ģʽ. \r\n");
//	mpu->set_gyro_offset(mpu);
	mpu->init_flag = 1;

	return 0;
}

/**
  ******************************************************************************
  * @brief   дMPU6050һ���ֽ�
  * @param   reg : MPU6050�ڲ��Ĵ�����ַ�� dat Ҫд���ֵ
  * @retval  0 д�ɹ��� 1 дʧ��
  ******************************************************************************
  */
static int32_t mpu6050_write_reg(mpu6050_t *mpu, uint8_t reg, uint8_t val)
{
	i2c_bus_t *i2c = mpu->i2c;
	
	assert(mpu);
//	mutex_lock(mpu->mutex);
	i2c->start(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1| I2C_WR);
	if(i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, reg);
	if(i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, val);
	if(i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->stop(i2c);
//	mutex_unlock(mpu->mutex);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
//	mutex_unlock(mpu->mutex);
	return -1;
}

/**
  ******************************************************************************
  * @brief   ��MPU6050һ���ֽ�
  * @param   reg : MPU6050�ڲ��Ĵ�����ַ
  * @retval  ������ֵ
  ******************************************************************************
  */
static int32_t mpu6050_read_reg(mpu6050_t *mpu, uint8_t reg, uint8_t *val)
{	
	i2c_bus_t *i2c = mpu->i2c;
	
	assert(mpu);
//	mutex_lock(mpu->mutex);
	i2c->start(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, reg);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->restart(i2c);							/* generate Sr condition. */
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->recv_byte(i2c, val);
	i2c->send_nack(i2c);
	i2c->stop(i2c);
//	mutex_unlock(mpu->mutex);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
//	mutex_unlock(mpu->mutex);
	return -1;
}

/**
  ******************************************************************************
  * @brief   ��MPU6050�еļ��ٶȡ������ǡ��¶�ֵ����������mpu_buffer������
  * @param   None
  * @retval  0 ��ȡ�ɹ��� 1 ��ȡʧ��
  ******************************************************************************
  */
int32_t mpu6050_read_raw_data(mpu6050_t *mpu)
{
	uint8_t i;
	i2c_bus_t *i2c = mpu->i2c;
	
//	mutex_lock(mpu->mutex);
	i2c->start(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, ACCEL_XOUT_H);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->restart(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	for (i = 0; i < 13; i++) {
		i2c->recv_byte(i2c, &mpu->buffer[i]);
		i2c->send_ack(i2c);
	}
	i2c->recv_byte(i2c, &mpu->buffer[13]);
	i2c->send_nack(i2c);
	i2c->stop(i2c);
//	mutex_unlock(mpu->mutex);
	return 0;	

i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
//	mutex_unlock(mpu->mutex);
	return -1;
}

int32_t mpu6050_read_data_to_buffer(mpu6050_t *mpu, char *buffer, uint32_t size, uint32_t *pos)
{
	uint8_t i;
	i2c_bus_t *i2c = mpu->i2c;
	
	if (*pos + 14 >= size) {
		debug("mpu buffer is full!\r\n");
		return -1;
	}
	
	i2c->start(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, ACCEL_XOUT_H);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->restart(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_RD);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	
	for (i = 0; i < 13; i++) {
		i2c->recv_byte(i2c, (uint8_t*)buffer+*pos);
		*pos++;
		i2c->send_ack(i2c);
	}
	i2c->recv_byte(i2c, (uint8_t*)buffer+*pos);
	*pos++;
	i2c->send_nack(i2c);
	i2c->stop(i2c);
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;
}

int32_t mpu6050_read_data(mpu6050_t *mpu)
{
	assert(mpu);
	if (mpu6050_read_raw_data(mpu) != 0) {
		err("mpu6050 read raw data failed!\r\n");
		return -1;
	}
	
	mpu->acce_last.x = ((((int16_t)mpu->buffer[0])  << 8) | mpu->buffer[1])  - mpu->offset_acce.x;
	mpu->acce_last.y = ((((int16_t)mpu->buffer[2])  << 8) | mpu->buffer[3])  - mpu->offset_acce.y;
	mpu->acce_last.z = ((((int16_t)mpu->buffer[4])  << 8) | mpu->buffer[5])  - mpu->offset_acce.z;
	mpu->gyro_last.x = ((((int16_t)mpu->buffer[8])  << 8) | mpu->buffer[9])  - mpu->offset_gyro.x;
	mpu->gyro_last.y = ((((int16_t)mpu->buffer[10]) << 8) | mpu->buffer[11]) - mpu->offset_gyro.y;
	mpu->gyro_last.z = ((((int16_t)mpu->buffer[12]) << 8) | mpu->buffer[13]) - mpu->offset_gyro.z;
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   config the gyroscopes' full scale range 
  * @param   param 	fsr 	Full Scale Range
  *					0		+-250��/S
  *					1		+-500��/S
  *					2		+-1000��/S
  *					3		+-2000��/S
  * @retval  0 ���óɹ��� 1 ����ʧ��
  ******************************************************************************
  */
static int32_t mpu6050_set_gyro_fsr(mpu6050_t *mpu, mpu6050_gyro_fsr_t fsr)
{
	uint8_t tmp;
	assert(mpu);
	switch (fsr) {
	case GYRO_FSR_250:
		mpu->gyro_fsr = GYRO_FSR_250;
		tmp = 0;
		break;
	case GYRO_FSR_500:
		mpu->gyro_fsr = GYRO_FSR_500;
		tmp = 1;
		break;
	case GYRO_FSR_1000:
		mpu->gyro_fsr = GYRO_FSR_1000;
		tmp = 2;
		break;
	case GYRO_FSR_2000:
		mpu->gyro_fsr = GYRO_FSR_2000;
		tmp = 3;
		break;
	default:
		err("cmd: MPU6050_CMD_SET_GYRO_FSR has no val: %d\r\n", fsr);
		return -1;
	}
	if (mpu->write_reg(mpu, GYRO_CONFIG, tmp << 3) != 0) {
		err("mpu6050_ioctl failed!\r\n");
		return -1;
	}	
	return 0;
}

/**
  ******************************************************************************
  * @brief   config the accelerometers' full scale range  
  * @param   param 	fsr 	Full Scale Range
  *					0		+-2g
  *					1		+-4g
  *					2		+-8g
  *					3		+-16g
  * @retval  0 ���óɹ��� 1 ����ʧ��
  ******************************************************************************
  */
static int32_t mpu6050_set_acce_fsr(mpu6050_t *mpu, mpu6050_acc_fsr_t fsr)
{
	uint8_t tmp;
	assert(mpu);
	switch (fsr) {
	case ACCEL_FSR_2g:
		mpu->acce_fsr = ACCEL_FSR_2g;
		tmp = 0;
		break;
	case ACCEL_FSR_4g:
		mpu->acce_fsr = ACCEL_FSR_4g;
		tmp = 1;
		break;
	case ACCEL_FSR_8g:
		mpu->acce_fsr = ACCEL_FSR_8g;
		tmp = 2;
		break;
	case ACCEL_FSR_16g:
		mpu->acce_fsr = ACCEL_FSR_16g;
		tmp = 3;
		break;
	default:
		err("cmd: MPU6050_CMD_SET_ACCE_FSR has no val: %d\r\n", fsr);
		return -1;
	}
	if (mpu->write_reg(mpu, ACCEL_CONFIG, tmp << 3) != 0) {
		err("mpu6050_ioctl failed!\r\n");
		return -1;
	}
	return 0;
}


/**
  ******************************************************************************
  * @brief  config the digital low pass filter Badnwidth  
  * @param  lpf   low pass filter 
  *   +-----------------------------------------------------------------------------------------------------------+
  *	  |		lpf 	Bandwidth_Accel(Hz) 	Delay(ms)			Bandwidth_Gyro(Hz)		Delay(ms)		Fs(KHz)	  |
  *	  |		0			260						0					256					0.98				8	  |
  *	  |		1			184						2.0					188					1.9					1	  |
  *	  |		2			94						3.0					98					2.8					1	  |	
  *	  |		3			44						4.9					42					4.8					1	  |
  *	  |		4			21						8.5					20					8.3					1 	  |
  *	  |		5			10						13.8				10					13.4				1	  |
  *	  |		6			5						19.0				5					18.6				1  	  |
  *	  |				RESERVED									RESERVED									8	  |
  *   +-----------------------------------------------------------------------------------------------------------+
  * @retval  0 ���óɹ��� 1 ����ʧ��
  ******************************************************************************
  */
static int32_t mpu6050_set_lpf(mpu6050_t *mpu, mpu6050_lpf_t lpf)
{
	uint8_t tmp;
	assert(mpu);
	switch (lpf) {
	case LPF_5Hz:
		tmp = 6;
		mpu->lpf = LPF_5Hz;
		break;
	case LPF_10Hz:
		tmp = 5;
		mpu->lpf = LPF_10Hz;
		break;
	case LPF_20Hz:
		tmp = 4;
		mpu->lpf = LPF_20Hz;
		break;
	case LPF_40Hz:
		tmp = 3;
		mpu->lpf = LPF_40Hz;
		break;
	case LPF_90Hz:
		tmp = 2;
		mpu->lpf = LPF_90Hz;
		break;
	case LPF_180Hz:
		tmp = 1;
		mpu->lpf = LPF_180Hz;
		break;
	case LPF_250Hz:
		tmp = 0;
		mpu->lpf = LPF_250Hz;
		break;
	default:
		err("cmd: MPU6050_CMD_SET_LPF has no val: %d\r\n", lpf);
		return -1;
	}
	if (mpu->write_reg(mpu, CONFIG, tmp) != 0) {
		err("mpu6050_ioctl failed!\r\n");
		return -1;
	}
	return 0;
}
/**
  ******************************************************************************
  * @brief   config sample rate
  * @param   SAMPLE_RATE_125Hz 250Hz 500Hz 1000Hz
  *          Sample Rate = Gyroscope Out Rate / (1 + SMPLRT_DIV)
  * 		 SMPLRT_DIV = 1000 / sample_rate - 1
  *  		   	maxmum sample rate is 1000Hz 
  * 			minmum sample rate is 4 Hz
  * @retval  0 ��ȡ�ɹ��� 1 ��ȡʧ��
  ******************************************************************************
  */
static int32_t mpu6050_set_sample_rate(mpu6050_t *mpu, mpu6050_sample_rate_t sr)
{
	uint8_t tmp;
	assert(mpu);
	switch (sr) {
	case SAMPLE_RATE_125Hz:
		mpu->sr = SAMPLE_RATE_125Hz;
		tmp = 7;
		break;
	case SAMPLE_RATE_250Hz:
		mpu->sr = SAMPLE_RATE_250Hz;
		tmp = 3;
		break;
	case SAMPLE_RATE_500Hz:
		mpu->sr = SAMPLE_RATE_500Hz;
		tmp = 1;
		break;
	case SAMPLE_RATE_1000Hz:
		mpu->sr = SAMPLE_RATE_1000Hz;
		tmp = 0;
		break;
	default:
		err("cmd: MPU6050_CMD_SET_SAMPLE_RATE has no val: %d\r\n", sr);
		return -1;
	}
	if (mpu->write_reg(mpu, SMPLRT_DIV, tmp) != 0) {
		err("mpu6050_ioctl failed!\r\n");
		return -1;
	}
	return 0;
}

/**
  ******************************************************************************
  * @brief   ����������Ưֵ
  * @param   None
  * @retval  None
  ******************************************************************************
  */


/**
  ******************************************************************************
  * @brief   MPU6050���ݴ��������Լ��ٶ����ݽ��л����˲�
  * @param   None
  * @retval  None
  ******************************************************************************
  */


/**
  ******************************************************************************
  * @brief   �����ݴ�ӡ������1
  * @param   None
  * @retval  None
  ******************************************************************************
  */

int32_t mpu6050_ioctl(mpu6050_t *mpu, uint8_t cmd, int32_t val)
{
	assert(mpu);
	switch (cmd) {
	case MPU6050_CMD_SET_GYRO_FSR:
		if (mpu6050_set_gyro_fsr(mpu, (mpu6050_gyro_fsr_t)val) != 0) {
			err("mpu6050 set gyro fsr failed!\r\n");
			return -1;
		}
		break;
		
	case MPU6050_CMD_SET_ACCE_FSR:
		if (mpu6050_set_acce_fsr(mpu, (mpu6050_acc_fsr_t)val) != 0) {
			err("mpu6050 set acce fsr failed!\r\n");
			return -1;
		}
		break;
		
	case MPU6050_CMD_SET_LPF:
		if (mpu6050_set_lpf(mpu, (mpu6050_lpf_t)val) != 0) {
			err("mpu6050 set lpf failed!\r\n");
			return -1;
		}	
		break;
		
	case MPU6050_CMD_SET_SAMPLE_RATE:
		if (mpu6050_set_sample_rate(mpu, (mpu6050_sample_rate_t)val) != 0) {
			err("mpu6050 set sample rate failed!\r\n");
			return -1;
		}
		break;
		
	default:
		err("mpu6050 no such command!\r\n");
		return -1;
	}
	
	return 0;
}

int32_t mpu6050_check_device(mpu6050_t *mpu)
{
	i2c_bus_t *i2c = mpu->i2c;
	
	assert(mpu);
	i2c->start(i2c);
	i2c->send_byte(i2c, MPU6050_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	
	i2c->stop(i2c);
	debug(DEVICE "mpu6050 found!\r\n");
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("No mpu6050 device found!\r\n");
	return -1;
}


int32_t mpu6050_register(mpu6050_t *mpu, i2c_bus_t *i2c)
{
	assert(mpu);
	assert(i2c);
//	mpu->mutex = mutex_create("MPU6050");
	mpu->i2c = i2c;
	mpu->check_device = mpu6050_check_device;
	mpu->init = mpu6050_init;
	mpu->read_data = mpu6050_read_data;
	mpu->ioctl = mpu6050_ioctl;
	mpu->write_reg = mpu6050_write_reg;
	mpu->read_reg = mpu6050_read_reg;

	return 0;
}



#include "wm8978.h"
#include "config.h"

#define WM8978_ADDR				0X1A

#undef 	DEVICE
#define DEVICE 					"[WM8978]: "

/* global variable. */
wm8978_t  wm8978;

/*
 * �����ļ��ڲ����õĺ���
*/
static int32_t wm8978_write_reg(wm8978_t *wm, uint8_t reg, uint16_t val);
static int32_t wm8978_read_reg(wm8978_t *wm, uint8_t reg, uint16_t *val);
static int32_t wm8978_set_adc(wm8978_t *wm, void *val);
static int32_t wm8978_set_dac(wm8978_t *wm, void *val);
static int32_t wm8978_set_input(wm8978_t *wm, void *val);
static int32_t wm8978_set_output(wm8978_t *wm, void *val);
static int32_t wm8978_set_linein_gain(wm8978_t *wm, void *val);
static int32_t wm8978_set_aux_gain(wm8978_t *wm, void *val);
static int32_t wm8978_set_mic_gain(wm8978_t *wm, void *val);
static int32_t wm8978_set_mic(wm8978_t *wm, void *val);
static int32_t wm8978_set_speaker(wm8978_t *wm, void *val);
static int32_t wm8978_set_headset(wm8978_t *wm, void *val);
static int32_t wm8978_set_headset_volume(wm8978_t *wm, void *val);
static int32_t wm8978_set_speaker_volume(wm8978_t *wm, void *val);
static int32_t wm8978_set_3d(wm8978_t *wm, void *val);
static int32_t wm8978_set_eq_3d_dir(wm8978_t *wm, void *val);
static int32_t wm8978_set_i2s_config(wm8978_t *wm, void *val);
static int32_t wm8978_set_recorder_mode(wm8978_t *wm);
static int32_t wm8978_set_player_mode(wm8978_t *wm);

int32_t wm8978_check_device(wm8978_t *wm);
int32_t wm8978_init(wm8978_t *wm);
int32_t wm8978_ioctl(wm8978_t *wm, uint8_t cmd, void *val);

static uint16_t wm8978_reg_ram_init[58] = {
		0X0000, 0X0000, 0X0000, 0X0000, 0X0050, 0X0000, 0X0140, 0X0000,
		0X0000, 0X0000, 0X0000, 0X00FF, 0X00FF, 0X0000, 0X0100, 0X00FF,
		0X00FF, 0X0000, 0X012C, 0X002C, 0X002C, 0X002C, 0X002C, 0X0000,
		0X0032, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000, 0X0000,
		0X0038, 0X000B, 0X0032, 0X0000, 0X0008, 0X000C, 0X0093, 0X00E9,
		0X0000, 0X0000, 0X0000, 0X0000, 0X0003, 0X0010, 0X0010, 0X0100,
		0X0100, 0X0002, 0X0001, 0X0001, 0X0039, 0X0039, 0X0039, 0X0039,
		0X0001, 0X0001,
};

/*
 * WM8978�豸ע��
 */
int32_t wm8978_register(wm8978_t *wm, i2c_bus_t *i2c)
{
	assert(wm);
	assert(i2c);
	wm->i2c 			= i2c;
	wm->check_device 	= wm8978_check_device;
	wm->init 			= wm8978_init;
	wm->ioctl 			= wm8978_ioctl;
	return 0;
}

/* 
 * ���WM8978�Ƿ����I2C������ 
 */
int32_t wm8978_check_device(wm8978_t *wm)
{
	i2c_bus_t *i2c = wm->i2c;
	
	assert(wm);
	i2c->start(i2c);
	i2c->send_byte(i2c, WM8978_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK) 
		goto i2c_err;
	
	i2c->stop(i2c);
	debug(DEVICE "I2C�����Ϸ���WM8978�豸��\r\n");
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("I2C������δ����WM8978�豸!\r\n");
	return -1;
}

/*
 * WM8978�豸��ʼ��
*/
int32_t wm8978_init(wm8978_t *wm)
{
	uint8_t i;
	
	assert(wm);
	for (i = 0; i < 58; i++) 
		wm->reg_ram[i] = wm8978_reg_ram_init[i];
	
	wm8978_write_reg(wm,  0, 0x000);			/* ��λWM8978 */
	wm8978_write_reg(wm,  1, 0x01b);			/* MICEN����Ϊ1(MICʹ��),BIASEN����Ϊ1(ģ��������),VMIDSEL[1:0]����Ϊ:11(5K) */
	wm8978_write_reg(wm,  2, 0x1b0);			/* ROUT1,LOUT1���ʹ��(�������Թ���),BOOSTENR,BOOSTENLʹ�� */
	wm8978_write_reg(wm,  3, 0x06c);			/* LOUT2,ROUT2���ʹ��(���ȹ���),RMIX,LMIXʹ�� */
	wm8978_write_reg(wm,  6, 0x000);			/* MCLK���ⲿ�ṩ */
	wm8978_write_reg(wm, 43, 1 << 4);			/* INVROUT2����,�������� */
	wm8978_write_reg(wm, 47, 1 << 8);			/* PGABOOSTL,��ͨ��MIC���20������ */
	wm8978_write_reg(wm, 48, 1 << 8);			/* PGABOOSTR,��ͨ��MIC���20������ */
	wm8978_write_reg(wm, 49, 1 << 1);			/* TSDEN,�������ȱ��� */
	wm8978_write_reg(wm, 10, 1 << 3);			/* SOFTMUTE�ر�,128x����,���SNR */
	wm8978_write_reg(wm, 14, 1 << 3);			/* ADC 128x������ */
	
	debug(DEVICE "WM8978 ��ʼ���ɹ�!\r\n");
	return 0;
}

/*
#define WM8978_CMD_SET_ADC  				1				ADC ����
#define WM8978_CMD_SET_DAC 					2				DAC ����
#define WM8978_CMD_SET_INPUT 				3				��������
#define WM8978_CMD_SET_OUTPUT    			4				�������
#define WM8978_CMD_SET_LINEIN_GAIN 			5				Linein ����
#define WM8978_CMD_SET_AUX_GAIN 			6				Aux    ����
#define WM8978_CMD_SET_MIC_GAIN 			7				Mic	   ����
#define WM8978_CMD_SET_MIC					8				mic    ����
#define WM8978_CMD_SET_SPEAKER				9 		        Speaker ����
#define WM8978_CMD_SET_HEADSET				10 				Headset ����
#define WM8978_CMD_SET_HEADSET_VOLUME 		11 				Headset ����
#define WM8978_CMD_SET_SPEAKER_VOLUME 		12 				Speaker ����
#define WM8978_CMD_SET_3D					13              3D��Ч	����
#define WM8978_CMD_SET_EQ_3D_DIR			14   			3D
#define WM8978_CMD_SET_PLAYER_MODE			20 				����Ϊ����ģʽ
#define WM8978_CMD_SET_RECORDER_MODE 		21				����Ϊ¼��ģʽ
*/
int32_t wm8978_ioctl(wm8978_t *wm, uint8_t cmd, void *val)
{
	assert(wm);
	switch (cmd) {
	case WM8978_CMD_SET_ADC:
		wm8978_set_adc(wm, val);
		break;
	case WM8978_CMD_SET_DAC:
		wm8978_set_dac(wm, val);
		break;
	case WM8978_CMD_SET_INPUT:
		wm8978_set_input(wm, val);
		break;
	case WM8978_CMD_SET_OUTPUT:
		wm8978_set_output(wm, val);
		break;
	case WM8978_CMD_SET_LINEIN_GAIN:
		wm8978_set_linein_gain(wm, val);
		break;
	case WM8978_CMD_SET_AUX_GAIN:
		wm8978_set_aux_gain(wm, val);
		break;
	case WM8978_CMD_SET_MIC_GAIN:
		wm8978_set_mic_gain(wm, val);
		break;
	case WM8978_CMD_SET_MIC:
		wm8978_set_mic(wm, val);
		break;
	case WM8978_CMD_SET_SPEAKER:
		wm8978_set_speaker(wm, val);
		break;
	case WM8978_CMD_SET_HEADSET:
		wm8978_set_headset(wm, val);
		break;
	case WM8978_CMD_SET_HEADSET_VOLUME:
		wm8978_set_headset_volume(wm, val);
		break;
	case WM8978_CMD_SET_SPEAKER_VOLUME:
		wm8978_set_speaker_volume(wm, val);
		break;
	case WM8978_CMD_SET_3D:
		wm8978_set_3d(wm, val);
		break;
	case WM8978_CMD_SET_EQ_3D_DIR:
		wm8978_set_eq_3d_dir(wm, val);
		break;
	case WM8978_CMD_SET_I2S_CONFIG:
		wm8978_set_i2s_config(wm, val);
		break;
	case WM8978_CMD_SET_PLAYER_MODE:
		wm8978_set_player_mode(wm);
		break;
	case WM8978_CMD_SET_RECORDER_MODE:
		wm8978_set_recorder_mode(wm);
		break;
	default:
		err("wm8978 no such cmd, cmd: %d\r\n", cmd);
		return -1;
	}
	return 0;
}

/* 
 * д�����ֽڵ�WM8978ָ���ļĴ�����
*/
static int32_t wm8978_write_reg(wm8978_t *wm, uint8_t reg, uint16_t val)
{
	i2c_bus_t *i2c = wm->i2c;
	
	i2c->start(i2c);
	i2c->send_byte(i2c, WM8978_ADDR << 1 | I2C_WR);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, reg << 1 | ((val >> 8) & 0x01));
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->send_byte(i2c, val & 0xff);
	if (i2c->wait_ack(i2c) != I2C_ACK)
		goto i2c_err;
	i2c->stop(i2c);
	wm->reg_ram[reg] = val;		/* ����д��Ĵ�����ֵ������ʱ��ֱ�ӷ��� */
	return 0;

i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;
}

/*
 * ��ȡWM8978�Ĵ�����ʵ������ֱ�Ӷ�ȡ�����е����� 
 */
static int32_t wm8978_read_reg(wm8978_t *wm, uint8_t reg, uint16_t *val)
{
	assert(wm);
	assert(val);
	*val = wm->reg_ram[reg];
	return 0;
}

/*
 * ����WM8978Ϊ¼��ģʽ
*/
static int32_t wm8978_set_recorder_mode(wm8978_t *wm)
{
	int32_t val;
	wm8978_status_t 	status;
	wm8978_input_t 	input;
	wm8978_output_t 	output;
	
	val = 20;
	wm->ioctl(wm, WM8978_CMD_SET_HEADSET_VOLUME, &val);	/* ��������20 */
	val = 0;
	wm->ioctl(wm, WM8978_CMD_SET_SPEAKER_VOLUME, &val);	/* Speak����0 */
	status = WM8978_ON;
	wm->ioctl(wm, WM8978_CMD_SET_ADC, &status);			/* ��ADC */
	status = WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_DAC, &status);			/* �ر�DAC */
	
	input.mic = WM8978_ON;
	input.line = WM8978_ON;
	input.aux = WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_INPUT, &input);		/* ����MIC��LINEIN */
	
	output.dac = WM8978_OFF;
	output.bps = WM8978_ON;	
	wm->ioctl(wm, WM8978_CMD_SET_OUTPUT, &output);		/* �������ΪBYPASS */
	
	status = WM8978_ON;
	wm->ioctl(wm, WM8978_CMD_SET_MIC, &status);			/* ��MIC */
	
	val = 46;
	wm->ioctl(wm, WM8978_CMD_SET_MIC_GAIN, &val); 		/* ����Mic���� */
	wm->ioctl(wm, WM8978_CMD_SET_I2S_CONFIG, &val);		/* �����ֱ�׼I2S */
	
	return 0;
}

/*
 * ����ADC�������ǹر�, ¼����ʱ����Ҫ����
 * ������ R2 |= 3 << 0
 * �رգ� R2 &= ~(3 << 0)
 */
static int32_t wm8978_set_adc(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t adc = *(wm8978_status_t *)val;
	
	wm8978_read_reg(wm, 2, &regval);		/* ��ȡR2 */
	if (adc == WM8978_ON) {
		regval |= 3 << 0;					/* R2���2��λ����Ϊ1,����ADCR&ADCL������������ADC�� */
		debug(DEVICE "����ADC\r\n");
	}
	else {
		regval &= ~(3 << 0);				/* R2���2��λ����,�ر�ADCR&ADCL������������ADC�� */
		debug(DEVICE "�ر�ADC\r\n");
	}
	if (wm8978_write_reg(wm, 2, regval) != 0) {
		err("wm8978_set_adc failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * ����DAC�������ǹرգ� ������ʱ����Ҫ����
 * ������ R3 |= 3 << 0
 * �رգ� R3 &= ~(3 << 0)
 */
static int32_t wm8978_set_dac(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t dac = *(wm8978_status_t *)val;
	
	wm8978_read_reg(wm, 3, &regval);		/* ��ȡR3 */
	if (dac == WM8978_ON) {
		regval |= 3 << 0;					/* R3���2��λ����Ϊ1,����DACR&DACL */
		debug(DEVICE "����DAC\r\n");
	}
	else {
		debug(DEVICE "�ر�DAC\r\n");
		regval &= ~(3 << 0);				/* R3���2��λ����,�ر�DACR&DACL. */
	}
	if (wm8978_write_reg(wm, 3, regval) != 0) {
		err("wm8978_set_dac failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * ����WM8978����Ƶ����
 * ������Ƶ����: 1. MIC 2. LINE 3. AUX
*/
static int32_t wm8978_set_input(wm8978_t *wm, void *val)
{
	uint16_t regval;
	int32_t gain;
	
	wm8978_input_t *input = (wm8978_input_t *)val;
	wm8978_read_reg(wm, 2, &regval);	/* ��ȡR2 */
	if (input->mic == WM8978_ON) {
		regval |= 3 << 2;				/* ����INPPGAENR,INPPGAENL(MIC��PGA�Ŵ�) */
		debug(DEVICE "����INPPGAENR, INPPGAENL\r\n");
	}
	else {
		regval &= ~(3 << 2);			/* �ر�INPPGAENR,INPPGAENL */
		debug(DEVICE "�ر�INPPGAENR, INPPGAENL\r\n");
	}
		
	if (wm8978_write_reg(wm, 2, regval) != 0) {
		err("wm8978_set_input MIC PGA failed!\r\n");
		return -1;
	}
	
	wm8978_read_reg(wm, 44, &regval);	/* ��ȡR44 */
	if (input->mic == WM8978_ON) {
		regval |= 3 << 4 | 3 << 0;		/* ����LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA */
		debug(DEVICE "����LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA\r\n");
	}
	else {
		regval &= ~(3 << 4 | 3 << 0);	/* �ر�LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA */
		debug(DEVICE "�ر�LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA\r\n");
	}
	if (wm8978_write_reg(wm, 44, regval) != 0) {
		err("wm8978_set_input mic failed!\r\n");
		return -1;
	}
	
	if (input->line == WM8978_ON) {
		gain = 5;
		wm8978_set_linein_gain(wm, &gain);	/* LINE IN 0dB���� */
		debug(DEVICE "����LINE IN 0dB����\r\n");
	} else {
		gain = 0;
		wm8978_set_linein_gain(wm, &gain);	/* �ر�LINE IN */
		debug(DEVICE "�ر�LINE IN\r\n");
	}
	
	if (input->aux == WM8978_ON) {
		gain = 7;
		wm8978_set_aux_gain(wm, &gain);		/* AUX 6dB���� */
		debug(DEVICE "����AUX IN 0dB����\r\n");
	} else {
		gain = 0;
		wm8978_set_aux_gain(wm, &gain);		/* �ر�AUX����  */
		debug(DEVICE "�ر�AUX IN\r\n");
	}
	
	return 0;
}

/* 
 * ����WM8978����Ƶ��� 
 * dacen:DAC���(����)����(1)/�ر�(0)
 * bpsen:Bypass���(¼��,����MIC,LINE IN,AUX��)����(1)/�ر�(0) 
*/
static int32_t wm8978_set_output(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_output_t *output = (wm8978_output_t *)val;
	
	if (output->dac == WM8978_ON) 
		regval |= 1 << 0;					/* DAC���ʹ�� */
	if (output->bps == WM8978_ON) {			
		regval |= 1 << 1;					/* ByPassʹ�� */
		regval |= 5 << 2;					/* 0dB���� */
	}
	if (wm8978_write_reg(wm, 50, regval) != 0 || wm8978_write_reg(wm, 51, regval) != 0) {
		err("wm8978_set_output failed!\r\n");
		return -1;
	}
	return 0;	
}

/*
 * ����WM8978��Linein ����
*/
static int32_t wm8978_set_linein_gain(wm8978_t *wm, void *val)
{
	uint16_t regval;
	int32_t gain = *(int32_t *)val;
	
	gain &= 0x07;
	wm8978_read_reg(wm, 47, &regval);
	regval &= ~(7 << 4);
	if (wm8978_write_reg(wm, 47, regval | gain << 4) != 0) {
		err("wm8978_set_linein_gain failed!\r\n");
		return -1;
	}
	wm8978_read_reg(wm, 48, &regval);
	regval &= ~(7 << 4);
	if (wm8978_write_reg(wm, 48, regval | gain << 4) != 0) {
		err("wm8978_set_linein_gain failed!\r\n");
		return -1;
	}
	
	return 0;
}

/*
 * ����WM8978��Aux ����
*/
static int32_t wm8978_set_aux_gain(wm8978_t *wm, void *val)
{
	uint16_t regval;
	int32_t gain = *(int32_t *)val;
	
	wm8978_read_reg(wm, 47, &regval);
	regval &= ~(7 << 0);
	if (wm8978_write_reg(wm, 47, regval) != 0) {
		err("wm8978_set_aux_gain failed!\r\n");
		return -1;
	}
	wm8978_read_reg(wm, 48, &regval);
	regval &= ~(7 << 0);
	if (wm8978_write_reg(wm, 48, regval | gain << 0) != 0) {
		err("wm8978_set_aux_gain failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * ����WM8978��Mic ����
 * WM8978 MIC��������(������BOOST��20dB,MIC-->ADC���벿�ֵ�����)
 * gain:0~63,��Ӧ-12dB~35.25dB,0.75dB/Step
*/
static int32_t wm8978_set_mic_gain(wm8978_t *wm, void *val)
{
	int32_t gain = *(int32_t *)val;
	
	gain &= 0x3f;
	/* R45,��ͨ��PGA����, R46,��ͨ��PGA����  */
	if (wm8978_write_reg(wm, 45, gain) != 0 || wm8978_write_reg(wm, 46, gain | 1 << 8) != 0) {
		err("wm8978_set_mic_gain failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * �򿪹ر�Mic
*/
static int32_t wm8978_set_mic(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t status = *(wm8978_status_t *)val;
	
	wm8978_read_reg(wm, 2, &regval);
	if (status == WM8978_OFF) 
		regval &= ~(3 << 2);
	else 
		regval |= 3 << 2;
	
	if (wm8978_write_reg(wm, 2, regval) != 0) {
		err("wm8978_set_mic failed!\r\n");
		return -1;
	}
	return 0;
}

/* 
 * �򿪹ر�Speaker
*/
static int32_t wm8978_set_speaker(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t status = *(wm8978_status_t *)val;
	
	if (status == WM8978_OFF) 
		regval = 0;
	else
		regval = 1;
	
	if (wm8978_write_reg(wm, 50, regval) != 0) {
		err("wm8978_set_speaker failed!\r\n");
		return -1;
	}
	
	return 0;
}


/* 
 * �򿪹ر�Headset
*/
static int32_t wm8978_set_headset(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t status = *(wm8978_status_t *)val;
	
	if (status == WM8978_OFF) 
		regval = 0;
	else {
		regval |= 1 << 1;
		regval |= 5 << 2;
	}
	if (wm8978_write_reg(wm, 51, regval) != 0) {
		err("wm8978_set_speaker failed!\r\n");
		return -1;
	}
	
	return 0;
}


/* 
 * ����Headset ����
*/
static int32_t wm8978_set_headset_volume(wm8978_t *wm, void *val)
{
	int32_t volume = *(int32_t *)val;
	
	volume &= 0x3f;
	if (volume == 0)
		volume |= 1 << 6;		/* ����Ϊ0�� ֱ�Ӿ��� */
	if (wm8978_write_reg(wm, 52, volume) != 0 || wm8978_write_reg(wm, 53, volume | 1 << 8) != 0) {
		err("wm8978_set_headset_volume failed!\r\n");
		return -1;
	}	
	return 0;
}

static int32_t wm8978_set_speaker_volume(wm8978_t *wm, void *val)
{
	int32_t volume = *(int32_t *)volume;
	
	volume &= 0x3f;
	if (volume == 0) 
		volume |= 1 << 6;		/* ����Ϊ0�� ֱ�Ӿ��� */
	if (wm8978_write_reg(wm, 54, volume) != 0 || wm8978_write_reg(wm, 55, volume | 1 << 8) != 0) {
		err("wm8978_set_speaker_volume failed!\r\n");
		return -1;
	}
	return 0;
}


/*
 * ����3D���ƣ� 3Dǿ�ȣ� 15��ǿ
*/
static int32_t wm8978_set_3d(wm8978_t *wm, void *val)
{
	int32_t depth = *(int32_t *)val;
	depth &= 0x0f;
	if (wm8978_write_reg(wm, 41, depth) != 0) {
		err("wm8978_set_3d failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * ����EQ/3D���÷���
 */
static int32_t wm8978_set_eq_3d_dir(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t dir = *(wm8978_status_t*)val;
	
	wm8978_read_reg(wm, 0x12, &regval);
	if (dir == WM8978_OFF)
		regval &= ~(1 << 8);
	else
		regval |= 1 << 8;
	
	if (wm8978_write_reg(wm, 18, regval) != 0) {
		err("wm8978_set_eq_3d_dir failed!\r\n");
		return -1;
	}
	return 0;
}

/* 
 * �����ֱ�׼�� 16bit���� 
 */
static int32_t wm8978_set_i2s_config(wm8978_t *wm, void *val)
{
	if (wm8978_write_reg(wm, 4, 2 << 3 | 0 << 5) != 0) {
		err("wm8978_set_i2s_config\r\n");
		return -1;
	}
	debug(DEVICE "������16bit��׼\r\n");
	return 0;
}

/*
 * ����WM8978Ϊ������ģʽ
 */
static int32_t wm8978_set_player_mode(wm8978_t *wm)
{
	int32_t val;
	wm8978_status_t 	status;
	wm8978_input_t 	input;
	wm8978_output_t 	output;
	
	assert(wm);
	val 		= 20;
	wm->ioctl(wm, WM8978_CMD_SET_HEADSET_VOLUME, &val);	/* ��������20 */
	val 		= 0;
	wm->ioctl(wm, WM8978_CMD_SET_SPEAKER_VOLUME, &val);	/* Speak����0 */
	status 		= WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_ADC, &status);			/* �ر�ADC */
	status 		= WM8978_ON;
	wm->ioctl(wm, WM8978_CMD_SET_DAC, &status);			/* ��DAC */
	input.mic 	= WM8978_OFF;
	input.line 	= WM8978_OFF;
	input.aux 	= WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_INPUT, &input);		/* û������ */
	output.dac 	= WM8978_ON;
	output.bps 	= WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_OUTPUT, &output);		/* �������ΪDAC */
	wm->ioctl(wm, WM8978_CMD_SET_I2S_CONFIG, &val);		/* 2 �����ֱ�׼I2S */
	
	return 0;
}




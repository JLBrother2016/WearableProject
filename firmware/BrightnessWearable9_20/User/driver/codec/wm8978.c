#include "wm8978.h"
#include "config.h"

#define WM8978_ADDR				0X1A

#undef 	DEVICE
#define DEVICE 					"[WM8978]: "

/* global variable. */
wm8978_t  wm8978;

/*
 * 仅供文件内部调用的函数
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
 * WM8978设备注册
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
 * 检查WM8978是否挂载I2C总线上 
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
	debug(DEVICE "I2C总线上发现WM8978设备！\r\n");
	return 0;
	
i2c_err:
	i2c->stop(i2c);
	err("I2C总线上未发现WM8978设备!\r\n");
	return -1;
}

/*
 * WM8978设备初始化
*/
int32_t wm8978_init(wm8978_t *wm)
{
	uint8_t i;
	
	assert(wm);
	for (i = 0; i < 58; i++) 
		wm->reg_ram[i] = wm8978_reg_ram_init[i];
	
	wm8978_write_reg(wm,  0, 0x000);			/* 软复位WM8978 */
	wm8978_write_reg(wm,  1, 0x01b);			/* MICEN设置为1(MIC使能),BIASEN设置为1(模拟器工作),VMIDSEL[1:0]设置为:11(5K) */
	wm8978_write_reg(wm,  2, 0x1b0);			/* ROUT1,LOUT1输出使能(耳机可以工作),BOOSTENR,BOOSTENL使能 */
	wm8978_write_reg(wm,  3, 0x06c);			/* LOUT2,ROUT2输出使能(喇叭工作),RMIX,LMIX使能 */
	wm8978_write_reg(wm,  6, 0x000);			/* MCLK由外部提供 */
	wm8978_write_reg(wm, 43, 1 << 4);			/* INVROUT2反向,驱动喇叭 */
	wm8978_write_reg(wm, 47, 1 << 8);			/* PGABOOSTL,左通道MIC获得20倍增益 */
	wm8978_write_reg(wm, 48, 1 << 8);			/* PGABOOSTR,右通道MIC获得20倍增益 */
	wm8978_write_reg(wm, 49, 1 << 1);			/* TSDEN,开启过热保护 */
	wm8978_write_reg(wm, 10, 1 << 3);			/* SOFTMUTE关闭,128x采样,最佳SNR */
	wm8978_write_reg(wm, 14, 1 << 3);			/* ADC 128x采样率 */
	
	debug(DEVICE "WM8978 初始化成功!\r\n");
	return 0;
}

/*
#define WM8978_CMD_SET_ADC  				1				ADC 开关
#define WM8978_CMD_SET_DAC 					2				DAC 开关
#define WM8978_CMD_SET_INPUT 				3				配置输入
#define WM8978_CMD_SET_OUTPUT    			4				配置输出
#define WM8978_CMD_SET_LINEIN_GAIN 			5				Linein 增益
#define WM8978_CMD_SET_AUX_GAIN 			6				Aux    增益
#define WM8978_CMD_SET_MIC_GAIN 			7				Mic	   增益
#define WM8978_CMD_SET_MIC					8				mic    开关
#define WM8978_CMD_SET_SPEAKER				9 		        Speaker 开关
#define WM8978_CMD_SET_HEADSET				10 				Headset 开关
#define WM8978_CMD_SET_HEADSET_VOLUME 		11 				Headset 音量
#define WM8978_CMD_SET_SPEAKER_VOLUME 		12 				Speaker 音量
#define WM8978_CMD_SET_3D					13              3D音效	开关
#define WM8978_CMD_SET_EQ_3D_DIR			14   			3D
#define WM8978_CMD_SET_PLAYER_MODE			20 				设置为播音模式
#define WM8978_CMD_SET_RECORDER_MODE 		21				设置为录音模式
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
 * 写两个字节到WM8978指定的寄存器中
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
	wm->reg_ram[reg] = val;		/* 备份写入寄存器的值，读的时候直接返回 */
	return 0;

i2c_err:
	i2c->stop(i2c);
	err("i2c no ack!\r\n");
	return -1;
}

/*
 * 读取WM8978寄存器，实际上是直接读取缓存中的数据 
 */
static int32_t wm8978_read_reg(wm8978_t *wm, uint8_t reg, uint16_t *val)
{
	assert(wm);
	assert(val);
	*val = wm->reg_ram[reg];
	return 0;
}

/*
 * 设置WM8978为录音模式
*/
static int32_t wm8978_set_recorder_mode(wm8978_t *wm)
{
	int32_t val;
	wm8978_status_t 	status;
	wm8978_input_t 	input;
	wm8978_output_t 	output;
	
	val = 20;
	wm->ioctl(wm, WM8978_CMD_SET_HEADSET_VOLUME, &val);	/* 耳机音量20 */
	val = 0;
	wm->ioctl(wm, WM8978_CMD_SET_SPEAKER_VOLUME, &val);	/* Speak音量0 */
	status = WM8978_ON;
	wm->ioctl(wm, WM8978_CMD_SET_ADC, &status);			/* 打开ADC */
	status = WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_DAC, &status);			/* 关闭DAC */
	
	input.mic = WM8978_ON;
	input.line = WM8978_ON;
	input.aux = WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_INPUT, &input);		/* 开启MIC和LINEIN */
	
	output.dac = WM8978_OFF;
	output.bps = WM8978_ON;	
	wm->ioctl(wm, WM8978_CMD_SET_OUTPUT, &output);		/* 设置输出为BYPASS */
	
	status = WM8978_ON;
	wm->ioctl(wm, WM8978_CMD_SET_MIC, &status);			/* 打开MIC */
	
	val = 46;
	wm->ioctl(wm, WM8978_CMD_SET_MIC_GAIN, &val); 		/* 设置Mic增益 */
	wm->ioctl(wm, WM8978_CMD_SET_I2S_CONFIG, &val);		/* 飞利浦标准I2S */
	
	return 0;
}

/*
 * 设置ADC开启还是关闭, 录音的时候需要开启
 * 开启： R2 |= 3 << 0
 * 关闭： R2 &= ~(3 << 0)
 */
static int32_t wm8978_set_adc(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t adc = *(wm8978_status_t *)val;
	
	wm8978_read_reg(wm, 2, &regval);		/* 读取R2 */
	if (adc == WM8978_ON) {
		regval |= 3 << 0;					/* R2最低2个位设置为1,开启ADCR&ADCL（左右声道到ADC） */
		debug(DEVICE "开启ADC\r\n");
	}
	else {
		regval &= ~(3 << 0);				/* R2最低2个位清零,关闭ADCR&ADCL（左右声道到ADC） */
		debug(DEVICE "关闭ADC\r\n");
	}
	if (wm8978_write_reg(wm, 2, regval) != 0) {
		err("wm8978_set_adc failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * 设置DAC开启还是关闭， 放音的时候需要开启
 * 开启： R3 |= 3 << 0
 * 关闭： R3 &= ~(3 << 0)
 */
static int32_t wm8978_set_dac(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_status_t dac = *(wm8978_status_t *)val;
	
	wm8978_read_reg(wm, 3, &regval);		/* 读取R3 */
	if (dac == WM8978_ON) {
		regval |= 3 << 0;					/* R3最低2个位设置为1,开启DACR&DACL */
		debug(DEVICE "开启DAC\r\n");
	}
	else {
		debug(DEVICE "关闭DAC\r\n");
		regval &= ~(3 << 0);				/* R3最低2个位清零,关闭DACR&DACL. */
	}
	if (wm8978_write_reg(wm, 3, regval) != 0) {
		err("wm8978_set_dac failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * 配置WM8978的音频输入
 * 三种音频输入: 1. MIC 2. LINE 3. AUX
*/
static int32_t wm8978_set_input(wm8978_t *wm, void *val)
{
	uint16_t regval;
	int32_t gain;
	
	wm8978_input_t *input = (wm8978_input_t *)val;
	wm8978_read_reg(wm, 2, &regval);	/* 读取R2 */
	if (input->mic == WM8978_ON) {
		regval |= 3 << 2;				/* 开启INPPGAENR,INPPGAENL(MIC的PGA放大) */
		debug(DEVICE "开启INPPGAENR, INPPGAENL\r\n");
	}
	else {
		regval &= ~(3 << 2);			/* 关闭INPPGAENR,INPPGAENL */
		debug(DEVICE "关闭INPPGAENR, INPPGAENL\r\n");
	}
		
	if (wm8978_write_reg(wm, 2, regval) != 0) {
		err("wm8978_set_input MIC PGA failed!\r\n");
		return -1;
	}
	
	wm8978_read_reg(wm, 44, &regval);	/* 读取R44 */
	if (input->mic == WM8978_ON) {
		regval |= 3 << 4 | 3 << 0;		/* 开启LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA */
		debug(DEVICE "开启LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA\r\n");
	}
	else {
		regval &= ~(3 << 4 | 3 << 0);	/* 关闭LIN2INPPGA,LIP2INPGA,RIN2INPPGA,RIP2INPGA */
		debug(DEVICE "关闭LIN2INPPGA, LIP2INPGA, RIN2INPPGA, RIP2INPGA\r\n");
	}
	if (wm8978_write_reg(wm, 44, regval) != 0) {
		err("wm8978_set_input mic failed!\r\n");
		return -1;
	}
	
	if (input->line == WM8978_ON) {
		gain = 5;
		wm8978_set_linein_gain(wm, &gain);	/* LINE IN 0dB增益 */
		debug(DEVICE "开启LINE IN 0dB增益\r\n");
	} else {
		gain = 0;
		wm8978_set_linein_gain(wm, &gain);	/* 关闭LINE IN */
		debug(DEVICE "关闭LINE IN\r\n");
	}
	
	if (input->aux == WM8978_ON) {
		gain = 7;
		wm8978_set_aux_gain(wm, &gain);		/* AUX 6dB增益 */
		debug(DEVICE "开启AUX IN 0dB增益\r\n");
	} else {
		gain = 0;
		wm8978_set_aux_gain(wm, &gain);		/* 关闭AUX输入  */
		debug(DEVICE "关闭AUX IN\r\n");
	}
	
	return 0;
}

/* 
 * 配置WM8978的音频输出 
 * dacen:DAC输出(放音)开启(1)/关闭(0)
 * bpsen:Bypass输出(录音,包括MIC,LINE IN,AUX等)开启(1)/关闭(0) 
*/
static int32_t wm8978_set_output(wm8978_t *wm, void *val)
{
	uint16_t regval;
	wm8978_output_t *output = (wm8978_output_t *)val;
	
	if (output->dac == WM8978_ON) 
		regval |= 1 << 0;					/* DAC输出使能 */
	if (output->bps == WM8978_ON) {			
		regval |= 1 << 1;					/* ByPass使能 */
		regval |= 5 << 2;					/* 0dB增益 */
	}
	if (wm8978_write_reg(wm, 50, regval) != 0 || wm8978_write_reg(wm, 51, regval) != 0) {
		err("wm8978_set_output failed!\r\n");
		return -1;
	}
	return 0;	
}

/*
 * 设置WM8978的Linein 增益
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
 * 设置WM8978的Aux 增益
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
 * 设置WM8978的Mic 增益
 * WM8978 MIC增益设置(不包括BOOST的20dB,MIC-->ADC输入部分的增益)
 * gain:0~63,对应-12dB~35.25dB,0.75dB/Step
*/
static int32_t wm8978_set_mic_gain(wm8978_t *wm, void *val)
{
	int32_t gain = *(int32_t *)val;
	
	gain &= 0x3f;
	/* R45,左通道PGA设置, R46,右通道PGA设置  */
	if (wm8978_write_reg(wm, 45, gain) != 0 || wm8978_write_reg(wm, 46, gain | 1 << 8) != 0) {
		err("wm8978_set_mic_gain failed!\r\n");
		return -1;
	}
	return 0;
}

/*
 * 打开关闭Mic
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
 * 打开关闭Speaker
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
 * 打开关闭Headset
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
 * 设置Headset 音量
*/
static int32_t wm8978_set_headset_volume(wm8978_t *wm, void *val)
{
	int32_t volume = *(int32_t *)val;
	
	volume &= 0x3f;
	if (volume == 0)
		volume |= 1 << 6;		/* 音量为0， 直接静音 */
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
		volume |= 1 << 6;		/* 音量为0， 直接静音 */
	if (wm8978_write_reg(wm, 54, volume) != 0 || wm8978_write_reg(wm, 55, volume | 1 << 8) != 0) {
		err("wm8978_set_speaker_volume failed!\r\n");
		return -1;
	}
	return 0;
}


/*
 * 设置3D环绕， 3D强度， 15最强
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
 * 设置EQ/3D作用方向
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
 * 飞利浦标准， 16bit长度 
 */
static int32_t wm8978_set_i2s_config(wm8978_t *wm, void *val)
{
	if (wm8978_write_reg(wm, 4, 2 << 3 | 0 << 5) != 0) {
		err("wm8978_set_i2s_config\r\n");
		return -1;
	}
	debug(DEVICE "飞利浦16bit标准\r\n");
	return 0;
}

/*
 * 设置WM8978为播放器模式
 */
static int32_t wm8978_set_player_mode(wm8978_t *wm)
{
	int32_t val;
	wm8978_status_t 	status;
	wm8978_input_t 	input;
	wm8978_output_t 	output;
	
	assert(wm);
	val 		= 20;
	wm->ioctl(wm, WM8978_CMD_SET_HEADSET_VOLUME, &val);	/* 耳机音量20 */
	val 		= 0;
	wm->ioctl(wm, WM8978_CMD_SET_SPEAKER_VOLUME, &val);	/* Speak音量0 */
	status 		= WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_ADC, &status);			/* 关闭ADC */
	status 		= WM8978_ON;
	wm->ioctl(wm, WM8978_CMD_SET_DAC, &status);			/* 打开DAC */
	input.mic 	= WM8978_OFF;
	input.line 	= WM8978_OFF;
	input.aux 	= WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_INPUT, &input);		/* 没有输入 */
	output.dac 	= WM8978_ON;
	output.bps 	= WM8978_OFF;
	wm->ioctl(wm, WM8978_CMD_SET_OUTPUT, &output);		/* 设置输出为DAC */
	wm->ioctl(wm, WM8978_CMD_SET_I2S_CONFIG, &val);		/* 2 飞利浦标准I2S */
	
	return 0;
}




#ifndef _WM8978_H_
#define _WM8978_H_

#include "i2c.h"

#define WM8978_CMD_SET_ADC  				1
#define WM8978_CMD_SET_DAC 					2
#define WM8978_CMD_SET_INPUT 				3
#define WM8978_CMD_SET_OUTPUT    			4
#define WM8978_CMD_SET_LINEIN_GAIN 			5
#define WM8978_CMD_SET_AUX_GAIN 			6
#define WM8978_CMD_SET_MIC_GAIN 			7
#define WM8978_CMD_SET_MIC					8
#define WM8978_CMD_SET_SPEAKER				9
#define WM8978_CMD_SET_HEADSET				10
#define WM8978_CMD_SET_HEADSET_VOLUME 		11
#define WM8978_CMD_SET_SPEAKER_VOLUME 		12
#define WM8978_CMD_SET_3D					13
#define WM8978_CMD_SET_EQ_3D_DIR			14
#define WM8978_CMD_SET_I2S_CONFIG 			15
#define WM8978_CMD_SET_PLAYER_MODE			20
#define WM8978_CMD_SET_RECORDER_MODE 		21

typedef enum {
	WM8978_OFF = 0,
	WM8978_ON  = 1
} wm8978_status_t;

typedef struct wm8978_input_s {
	wm8978_status_t mic;
	wm8978_status_t line;
	wm8978_status_t aux;
} wm8978_input_t;

typedef struct wm8978_output_s {
	wm8978_status_t dac;
	wm8978_status_t bps;
} wm8978_output_t;

typedef struct wm8978 wm8978_t;
struct wm8978 {
	i2c_bus_t *i2c;
	int32_t (*check_device)(wm8978_t *dev);
	int32_t (*init)(wm8978_t *dev);
	int32_t (*ioctl)(wm8978_t *dev, uint8_t cmd, void *val);
	
/*	WM8978�Ĵ���ֵ������(�ܹ�58���Ĵ���,0~57),ռ��116�ֽ��ڴ�
 *	��ΪWM8978��I2C������֧�ֶ�����,�����ڱ��ر������мĴ���ֵ
 *	дWM8978�Ĵ���ʱ,ͬ�����µ����ؼĴ���ֵ,���Ĵ���ʱ,ֱ�ӷ��ر��ر���ļĴ���ֵ.
 *	WM8978�ļĴ���ֵ��9λ��,����Ҫ��uint16_t���洢.
*/
	uint16_t reg_ram[58];
};

extern wm8978_t  wm8978;
int32_t wm8978_register(wm8978_t *wm, i2c_bus_t *i2c);

#endif

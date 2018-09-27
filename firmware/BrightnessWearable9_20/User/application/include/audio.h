#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdint.h>
#include <ff.h>
#include "wav.h"
#include "wm8978.h"

/* ���ֲ��ſ����� */
typedef struct player player_t;
struct player {
	wavhead_t				*header;					/* wav hearder */
	uint8_t 				*buffer0;					/* i2s_dma��buf1�� ����DMA���͹��������� */
	uint8_t 				*buffer1;					/* i2s_dma��buf2�� ����DMA���͹��������� */
	volatile uint8_t 		buffer_flag; 				/* ����BUFFER��־�� 0�� ���dma_buffer0; 1: ���dma_buffer1 */
	volatile uint8_t 		transfered_flag;			/* WAV������ɱ�־ */
	FIL						*fp;						/* ��ǰ���ŵ���Ƶ�ļ�ָ�� */
	uint8_t 				state;						/* ��ǰ������Ƶ��״̬ */
														/* bit0 0��ͣ���ţ� 1�������� */
														/* bit1 0�������ţ� 1�������� */
	
	uint32_t 				tot_sec;					/* ¼����ʱ������λ�� */	
	uint32_t 				cur_sec;					/* ��ǰ���ŵ�ʱ���� ��λ�� */
	
	wm8978_t 				*wm;				
};


/* ¼�������� */
typedef struct recorder recorder_t;
struct recorder {
	wavhead_t				*header;		/* wav hearder */
	uint8_t 				*buffer0;		/* i2s_dma��buf1�� ����DMA���͹��������� */
	uint8_t 				*buffer1;		/* i2s_dma��buf2�� ����DMA���͹��������� */
	FIL						*fp;			/* ��ǰ���ŵ���Ƶ�ļ�ָ�� */
	uint32_t 				data_size;		/* ¼���ļ���С�� ������wavhead */
	uint8_t 				state;			/* ��ǰ������Ƶ��״̬ */
											/* bit0 0��ͣ¼���� 1����¼�� */
											/* bit1 0����¼���� 1����¼�� */
	wm8978_t 				*wm;
};


extern volatile player_t 		player_ctl;				/* ���������ƿ� */
extern volatile recorder_t 		recorder_ctl;				/* ���������ƿ� */

int32_t audio_set_play_mode(wm8978_t *wm);
int32_t audio_leave_play_mode(void);
int32_t audio_play(char *name);
int32_t audio_set_record_mode(wm8978_t *wm);
int32_t audio_leave_record_mode(void);
int32_t audio_record(char *name, uint32_t sec);
void recorder_start(void);
void recorder_stop(void);


#endif /* _AUDIO_H_ */

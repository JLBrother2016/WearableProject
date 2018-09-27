#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdint.h>
#include "ff.h"
#include "wm8978.h"


/* ���ֲ��ſ����� */
typedef struct player player_t;
struct player {
	uint8_t 		*buffer0;		/* i2s_dma��buf1�� ����DMA���͹��������� */
	uint8_t 		*buffer1;		/* i2s_dma��buf2�� ����DMA���͹��������� */
	
	FIL				*fp;			/* ��ǰ���ŵ���Ƶ�ļ�ָ�� */
	
	uint8_t 		state;			/* ��ǰ������Ƶ��״̬ */
									/* bit0 0��ͣ���ţ� 1�������� */
									/* bit1 0�������ţ� 1�������� */
};

int32_t play_audio(char *name, wm8978_t *wm);
void player_start(void);

#endif


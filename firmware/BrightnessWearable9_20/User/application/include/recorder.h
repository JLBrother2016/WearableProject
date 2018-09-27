#ifndef _RECORDER_H_
#define _RECORDER_H_


#include <stdint.h>
#include "ff.h"
#include "wm8978.h"

/* ¼�������� */
typedef struct recorder recorder_t;
struct recorder {
	uint8_t 		*buffer0;		/* i2s_dma��buf1�� ����DMA���͹��������� */
	uint8_t 		*buffer1;		/* i2s_dma��buf2�� ����DMA���͹��������� */
	FIL				*fp;			/* ��ǰ���ŵ���Ƶ�ļ�ָ�� */

	/* bit0 0��ͣ¼���� 1����¼�� */
	/* bit1 0����¼���� 1����¼�� */
	uint8_t 		state;			/* ��ǰ������Ƶ��״̬ */
	
	uint32_t 		data_size;		/* ¼���ļ���С�� ������wavhead */
									
									
};

#endif

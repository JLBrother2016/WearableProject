#ifndef _RECORDER_H_
#define _RECORDER_H_


#include <stdint.h>
#include "ff.h"
#include "wm8978.h"

/* 录音控制器 */
typedef struct recorder recorder_t;
struct recorder {
	uint8_t 		*buffer0;		/* i2s_dma的buf1， 保存DMA传送过来的数据 */
	uint8_t 		*buffer1;		/* i2s_dma的buf2， 保存DMA传送过来的数据 */
	FIL				*fp;			/* 当前播放的音频文件指针 */

	/* bit0 0暂停录音， 1继续录音 */
	/* bit1 0结束录音， 1开启录音 */
	uint8_t 		state;			/* 当前播放音频的状态 */
	
	uint32_t 		data_size;		/* 录音文件大小， 不包括wavhead */
									
									
};

#endif

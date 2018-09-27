#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <stdint.h>
#include "ff.h"
#include "wm8978.h"


/* 音乐播放控制器 */
typedef struct player player_t;
struct player {
	uint8_t 		*buffer0;		/* i2s_dma的buf1， 保存DMA传送过来的数据 */
	uint8_t 		*buffer1;		/* i2s_dma的buf2， 保存DMA传送过来的数据 */
	
	FIL				*fp;			/* 当前播放的音频文件指针 */
	
	uint8_t 		state;			/* 当前播放音频的状态 */
									/* bit0 0暂停播放， 1继续播放 */
									/* bit1 0结束播放， 1开启播放 */
};

int32_t play_audio(char *name, wm8978_t *wm);
void player_start(void);

#endif


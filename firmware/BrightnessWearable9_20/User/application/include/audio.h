#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <stdint.h>
#include <ff.h>
#include "wav.h"
#include "wm8978.h"

/* 音乐播放控制器 */
typedef struct player player_t;
struct player {
	wavhead_t				*header;					/* wav hearder */
	uint8_t 				*buffer0;					/* i2s_dma的buf1， 保存DMA传送过来的数据 */
	uint8_t 				*buffer1;					/* i2s_dma的buf2， 保存DMA传送过来的数据 */
	volatile uint8_t 		buffer_flag; 				/* 填充的BUFFER标志， 0： 填充dma_buffer0; 1: 填充dma_buffer1 */
	volatile uint8_t 		transfered_flag;			/* WAV传输完成标志 */
	FIL						*fp;						/* 当前播放的音频文件指针 */
	uint8_t 				state;						/* 当前播放音频的状态 */
														/* bit0 0暂停播放， 1继续播放 */
														/* bit1 0结束播放， 1开启播放 */
	
	uint32_t 				tot_sec;					/* 录音总时长，单位秒 */	
	uint32_t 				cur_sec;					/* 当前播放的时长， 单位秒 */
	
	wm8978_t 				*wm;				
};


/* 录音控制器 */
typedef struct recorder recorder_t;
struct recorder {
	wavhead_t				*header;		/* wav hearder */
	uint8_t 				*buffer0;		/* i2s_dma的buf1， 保存DMA传送过来的数据 */
	uint8_t 				*buffer1;		/* i2s_dma的buf2， 保存DMA传送过来的数据 */
	FIL						*fp;			/* 当前播放的音频文件指针 */
	uint32_t 				data_size;		/* 录音文件大小， 不包括wavhead */
	uint8_t 				state;			/* 当前播放音频的状态 */
											/* bit0 0暂停录音， 1继续录音 */
											/* bit1 0结束录音， 1开启录音 */
	wm8978_t 				*wm;
};


extern volatile player_t 		player_ctl;				/* 播放器控制块 */
extern volatile recorder_t 		recorder_ctl;				/* 播放器控制块 */

int32_t audio_set_play_mode(wm8978_t *wm);
int32_t audio_leave_play_mode(void);
int32_t audio_play(char *name);
int32_t audio_set_record_mode(wm8978_t *wm);
int32_t audio_leave_record_mode(void);
int32_t audio_record(char *name, uint32_t sec);
void recorder_start(void);
void recorder_stop(void);


#endif /* _AUDIO_H_ */

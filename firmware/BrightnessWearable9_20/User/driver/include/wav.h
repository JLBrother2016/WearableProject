#ifndef _WAV_H_
#define _WAV_H_

#include <stdint.h>
#include "ff.h"

#define WAV_HEADER_SIZE 		(44)					/* wav文件头有44字节 */

#define WAVE_STR 				(0x45564157)			/* "WAVE" */
#define DATA_STR				(0x61746164)			/* "data" */
#define RIFF_STR				(0x46464952)			/* "RIFF" */
#define FMT_STR					(0x20746D66)			/* "FMT " */
#define ALIGN_4 				(4)						/* 四字节对齐 */
#define ENCODE_PCM				(0x01)					/* PCM编码 */

/* RIFF 块描述符 */
struct chunk_riff {
	uint32_t id;				/* "RIFF"表示符合RIFF文件规范 */
	uint32_t size;				/* 从下一个地址（08H）到文件尾的总字节数，也就是整个文件大小-8字节 */
	uint32_t fmt;				/* "WAVE"表示WAV文件, 该格式下需要sub-chunk: 'fmt'和'data' */
};

/* FMT 子块, 
 * 描述音频数据的具体格式
 * 也就是如何解析data块的原始数据
 */
struct chunk_fmt {
	uint32_t id;				/* "FMT "波形格式表示，最后一个字符为空 */
	uint32_t size;				/* 从文件开始到上一个地址（OFH）的长度，一般为0010H */
	uint16_t fmt;				/* 音频编码格式; 0X01,表示线性PCM;0X11表示IMA ADPCM */
	uint16_t channels;			/* 通道数， 1: 单声道； 2: 双通道 */
	uint32_t sample_rate;		/* 采样率 */
	uint32_t byte_rate;		/* 码率：（采样率*量化的位数*通道数）/8 单位 KB/s*/
	uint16_t block_align;		/* 音频原始数据对齐的字节数（通道数 * 量化位数）/8 */
	uint16_t bits;				/* 音频幅值量化的位数 如16bits */			
};

/* 
 * DATA 子块 
 * 音频的原始数据
 */
struct chunk_data {
	uint32_t id;				/* "data"表示data块 */
	uint32_t size;				/* 音频数据部分的大小 */
};

/* WAV文件头结构 */
typedef struct wavhead wavhead_t;
struct wavhead {
	struct chunk_riff 	riff;	/* RIFF 块 */
	struct chunk_fmt 	fmt;	/* FMT  块 */
	struct chunk_data 	data;	/* DATA 块 */
};

typedef struct wavctl wav_ctl_t;
struct wavctl {
	uint16_t audio_fmt;				/* 音频编码格式; 0X01,表示线性PCM; 0X11表示IMA ADPCM */
	uint16_t channels;					/* 通道数量; 1,表示单声道; 2,表示双声道 */ 	
	uint16_t block_align;				/* 块对齐(字节) */
	
	uint32_t tot_sec;					/* 整首歌时长,单位:秒 */
	uint32_t cur_sec;					/* 当前播放时长 */
	
	uint32_t bit_rate;					/* 比特率(位速) byte_rate * 8 */
	uint32_t sample_rate;				/* 采样率  */
	uint16_t bits;						/* 位数, 比如16bit,24bit,32bit */
	
	uint32_t data_size;				/* 音频原始数据大小 */	
	uint32_t data_start;				/* 原始数据相对于文件开头的偏移(在文件里面的偏移) */
};


int32_t wavhead_encode(wavhead_t *head);
int32_t wavhead_decode(char *name, wavhead_t *head);
int32_t wavhead_write_data_size(FIL *fp, struct wavhead *head, uint32_t size);


#endif

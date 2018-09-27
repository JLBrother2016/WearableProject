#ifndef _WAV_H_
#define _WAV_H_

#include <stdint.h>
#include "ff.h"

#define WAV_HEADER_SIZE 		(44)					/* wav�ļ�ͷ��44�ֽ� */

#define WAVE_STR 				(0x45564157)			/* "WAVE" */
#define DATA_STR				(0x61746164)			/* "data" */
#define RIFF_STR				(0x46464952)			/* "RIFF" */
#define FMT_STR					(0x20746D66)			/* "FMT " */
#define ALIGN_4 				(4)						/* ���ֽڶ��� */
#define ENCODE_PCM				(0x01)					/* PCM���� */

/* RIFF �������� */
struct chunk_riff {
	uint32_t id;				/* "RIFF"��ʾ����RIFF�ļ��淶 */
	uint32_t size;				/* ����һ����ַ��08H�����ļ�β�����ֽ�����Ҳ���������ļ���С-8�ֽ� */
	uint32_t fmt;				/* "WAVE"��ʾWAV�ļ�, �ø�ʽ����Ҫsub-chunk: 'fmt'��'data' */
};

/* FMT �ӿ�, 
 * ������Ƶ���ݵľ����ʽ
 * Ҳ������ν���data���ԭʼ����
 */
struct chunk_fmt {
	uint32_t id;				/* "FMT "���θ�ʽ��ʾ�����һ���ַ�Ϊ�� */
	uint32_t size;				/* ���ļ���ʼ����һ����ַ��OFH���ĳ��ȣ�һ��Ϊ0010H */
	uint16_t fmt;				/* ��Ƶ�����ʽ; 0X01,��ʾ����PCM;0X11��ʾIMA ADPCM */
	uint16_t channels;			/* ͨ������ 1: �������� 2: ˫ͨ�� */
	uint32_t sample_rate;		/* ������ */
	uint32_t byte_rate;		/* ���ʣ���������*������λ��*ͨ������/8 ��λ KB/s*/
	uint16_t block_align;		/* ��Ƶԭʼ���ݶ�����ֽ�����ͨ���� * ����λ����/8 */
	uint16_t bits;				/* ��Ƶ��ֵ������λ�� ��16bits */			
};

/* 
 * DATA �ӿ� 
 * ��Ƶ��ԭʼ����
 */
struct chunk_data {
	uint32_t id;				/* "data"��ʾdata�� */
	uint32_t size;				/* ��Ƶ���ݲ��ֵĴ�С */
};

/* WAV�ļ�ͷ�ṹ */
typedef struct wavhead wavhead_t;
struct wavhead {
	struct chunk_riff 	riff;	/* RIFF �� */
	struct chunk_fmt 	fmt;	/* FMT  �� */
	struct chunk_data 	data;	/* DATA �� */
};

typedef struct wavctl wav_ctl_t;
struct wavctl {
	uint16_t audio_fmt;				/* ��Ƶ�����ʽ; 0X01,��ʾ����PCM; 0X11��ʾIMA ADPCM */
	uint16_t channels;					/* ͨ������; 1,��ʾ������; 2,��ʾ˫���� */ 	
	uint16_t block_align;				/* �����(�ֽ�) */
	
	uint32_t tot_sec;					/* ���׸�ʱ��,��λ:�� */
	uint32_t cur_sec;					/* ��ǰ����ʱ�� */
	
	uint32_t bit_rate;					/* ������(λ��) byte_rate * 8 */
	uint32_t sample_rate;				/* ������  */
	uint16_t bits;						/* λ��, ����16bit,24bit,32bit */
	
	uint32_t data_size;				/* ��Ƶԭʼ���ݴ�С */	
	uint32_t data_start;				/* ԭʼ����������ļ���ͷ��ƫ��(���ļ������ƫ��) */
};


int32_t wavhead_encode(wavhead_t *head);
int32_t wavhead_decode(char *name, wavhead_t *head);
int32_t wavhead_write_data_size(FIL *fp, struct wavhead *head, uint32_t size);


#endif

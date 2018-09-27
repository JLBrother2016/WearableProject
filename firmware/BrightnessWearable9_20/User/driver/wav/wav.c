#include "wav.h"
#include "config.h"


/*
 * 说明: 按照默认要求对录音文件进行wavheader编码
 */
int32_t wavhead_encode(wavhead_t *head)
{
	assert(head);
	head->riff.id 			= RIFF_STR;					/* "RIFF" */
	head->riff.size 		= 0;						/* 总文件大小-8字节, 之后来计算 */
	head->riff.fmt 			= WAVE_STR;					/* "WAVE" */
	head->fmt.id 			= FMT_STR;					/* "FMT " */
	head->fmt.size 			= 16;						/* 从文件开头到上一个地址的长度, 一般为16 */
	head->fmt.fmt 			= ENCODE_PCM;				/* PCM编码 */
	head->fmt.channels 		= 2;						/* 双声道 */
	head->fmt.sample_rate 	= 8000;					/* 采样率 8K */
	head->fmt.byte_rate 	= head->fmt.sample_rate * 4;/* 字节速率 = 采样率 * 通道数 * (ADC位数/8) */
	head->fmt.block_align 	= ALIGN_4;					/* 音频数据对齐 */
	head->fmt.bits 			= 16;						/* 量化的位数 */
	head->data.id 			= DATA_STR;					/* "data" */
	head->data.size 		= 0;						/* 原始数据大小, 之后来计算 */
	return 0;
}

/*
 * 将指定文件按照wavheader进行解析
 */
int32_t wavhead_decode(char *name, wavhead_t *head)
{
	FIL 				*fp;
	uint8_t 			*buf;
	FRESULT 			res;
	uint32_t 			br = 0;
	struct chunk_riff 	*riffp;
	struct chunk_fmt 	*fmtp;
	struct chunk_data 	*datap;
	
	fp = (FIL *)malloc(sizeof(FIL));
	buf = (uint8_t *)malloc(WAV_HEADER_SIZE); 		/* 保存读出wavheader 44 字节 */
	assert(fp);
	assert(buf);
	res = f_open(fp, name, FA_READ);
	if (res != FR_OK) {
		err("打开文件 %s 失败!\r\n", name);
		return -1;
	}
	
	f_read(fp, buf, WAV_HEADER_SIZE, &br);			/* 读取44字节， wavheader为44字节 */
	if (br != WAV_HEADER_SIZE) {
		err("读取%swav头出错！\r\n");
		return -1;
	}
	
	riffp = (struct chunk_riff *)buf;
	if(riffp->fmt != WAVE_STR) {					/* "WAVE" 是不是wav文件 */
		err("读到的文件不是wav文件.\r\n");
		return -1;
	}
	
	fmtp = (struct chunk_fmt *)(buf + 12);			/* fmt块的起始位置， 偏移量为12 */
	datap = (struct chunk_data *)(buf + 36);		/* data 块的起始位置， 偏移量为36 */
	if (datap->id != DATA_STR) {					/* "data" */ 
		err("解析WAV头文件失败!\r\n");
		return -1;
	}
	
	head->riff.id 			= riffp->id;
	head->riff.size			= riffp->size;
	head->riff.fmt			= riffp->fmt;
	head->fmt.id			= fmtp->id;
	head->fmt.size			= fmtp->size;
	head->fmt.fmt			= fmtp->fmt;
	head->fmt.channels		= fmtp->channels;
	head->fmt.sample_rate	= fmtp->sample_rate;
	head->fmt.byte_rate		= fmtp->byte_rate;
	head->fmt.block_align	= fmtp->block_align;
	head->fmt.bits			= fmtp->bits;
	head->data.id			= datap->id;
	head->data.size			= datap->size;
	
	f_close(fp);
	free(fp);
	free(buf);
	return 0;
}


/*
 * 说明: 录音结束后写入音频原始数据大小
 */
int32_t wavhead_write_data_size(FIL *fp, wavhead_t *head, uint32_t size)
{
	uint32_t bw;
	head->riff.size = size + 36;
	head->data.size = size;
	lseek(fp, 0);
	write(fp, (const void *)head, sizeof(struct wavhead), &bw);
	
	return 0;
}


/*
 * 说明: 打印解析到wav信息
 */
void wavhead_print_info(wavhead_t *info)
{
	if (info->fmt.fmt == ENCODE_PCM) 
		debug("\r\n音频编码方式: PCM\r\n");
	
	debug("AD量化位数: %d bit\r\n", info->fmt.bits);
	debug("声道数: %d\r\n", info->fmt.channels);
	debug("采样率: %d\r\n", info->fmt.sample_rate);
	debug("码率: %d bit/S\r\n", info->fmt.byte_rate);
	debug("音频原始数据对齐: %d Bytes\r\n", info->fmt.block_align);
	debug("音频数据大小: %d Bytes\r\n", info->data.size);
}



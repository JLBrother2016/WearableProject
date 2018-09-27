#include "wav.h"
#include "config.h"


/*
 * ˵��: ����Ĭ��Ҫ���¼���ļ�����wavheader����
 */
int32_t wavhead_encode(wavhead_t *head)
{
	assert(head);
	head->riff.id 			= RIFF_STR;					/* "RIFF" */
	head->riff.size 		= 0;						/* ���ļ���С-8�ֽ�, ֮�������� */
	head->riff.fmt 			= WAVE_STR;					/* "WAVE" */
	head->fmt.id 			= FMT_STR;					/* "FMT " */
	head->fmt.size 			= 16;						/* ���ļ���ͷ����һ����ַ�ĳ���, һ��Ϊ16 */
	head->fmt.fmt 			= ENCODE_PCM;				/* PCM���� */
	head->fmt.channels 		= 2;						/* ˫���� */
	head->fmt.sample_rate 	= 8000;					/* ������ 8K */
	head->fmt.byte_rate 	= head->fmt.sample_rate * 4;/* �ֽ����� = ������ * ͨ���� * (ADCλ��/8) */
	head->fmt.block_align 	= ALIGN_4;					/* ��Ƶ���ݶ��� */
	head->fmt.bits 			= 16;						/* ������λ�� */
	head->data.id 			= DATA_STR;					/* "data" */
	head->data.size 		= 0;						/* ԭʼ���ݴ�С, ֮�������� */
	return 0;
}

/*
 * ��ָ���ļ�����wavheader���н���
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
	buf = (uint8_t *)malloc(WAV_HEADER_SIZE); 		/* �������wavheader 44 �ֽ� */
	assert(fp);
	assert(buf);
	res = f_open(fp, name, FA_READ);
	if (res != FR_OK) {
		err("���ļ� %s ʧ��!\r\n", name);
		return -1;
	}
	
	f_read(fp, buf, WAV_HEADER_SIZE, &br);			/* ��ȡ44�ֽڣ� wavheaderΪ44�ֽ� */
	if (br != WAV_HEADER_SIZE) {
		err("��ȡ%swavͷ����\r\n");
		return -1;
	}
	
	riffp = (struct chunk_riff *)buf;
	if(riffp->fmt != WAVE_STR) {					/* "WAVE" �ǲ���wav�ļ� */
		err("�������ļ�����wav�ļ�.\r\n");
		return -1;
	}
	
	fmtp = (struct chunk_fmt *)(buf + 12);			/* fmt�����ʼλ�ã� ƫ����Ϊ12 */
	datap = (struct chunk_data *)(buf + 36);		/* data �����ʼλ�ã� ƫ����Ϊ36 */
	if (datap->id != DATA_STR) {					/* "data" */ 
		err("����WAVͷ�ļ�ʧ��!\r\n");
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
 * ˵��: ¼��������д����Ƶԭʼ���ݴ�С
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
 * ˵��: ��ӡ������wav��Ϣ
 */
void wavhead_print_info(wavhead_t *info)
{
	if (info->fmt.fmt == ENCODE_PCM) 
		debug("\r\n��Ƶ���뷽ʽ: PCM\r\n");
	
	debug("AD����λ��: %d bit\r\n", info->fmt.bits);
	debug("������: %d\r\n", info->fmt.channels);
	debug("������: %d\r\n", info->fmt.sample_rate);
	debug("����: %d bit/S\r\n", info->fmt.byte_rate);
	debug("��Ƶԭʼ���ݶ���: %d Bytes\r\n", info->fmt.block_align);
	debug("��Ƶ���ݴ�С: %d Bytes\r\n", info->data.size);
}



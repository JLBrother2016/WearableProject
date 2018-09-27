#include "audio.h"
#include "i2s.h"
#include "dma.h"
#include "config.h"
#include "fatfserr.h"



const uint16_t i2s_playbuf[2] = {0xdead, 0xbeef};		/* 2��16λ����,����¼��ʱI2S Master����.ѭ������0. */

volatile player_t 			player_ctl;				/* ���������ƿ� */
volatile recorder_t 			recorder_ctl;			/* ���������ƿ� */


void recorder_start(void);
void recorder_stop(void);
void player_start(void);						/* ���������� */
void player_stop(void);						/* ֹͣ������ */
void wav_get_cur_time(void);
void play_audio_i2s_dma_tx_callback(void);		/* I2S DMA TX �ص����� */
int32_t read_file_to_mem(uint8_t *buf, uint32_t size);
void print_audio_msg(void);

/* 
 * ���ֲ���ģʽ
 */
int32_t audio_set_play_mode(wm8978_t *wm)
{
	
	if (wm == NULL) {
		err("wm == NULL\r\n");
		return -1;
	}
	
	/* ��ʼ�����ֲ��ſ����� */
	player_ctl.wm = wm;
	player_ctl.buffer_flag = 0;
	player_ctl.transfered_flag = 0;
	player_ctl.state = 0;
	player_ctl.tot_sec = 0;
	player_ctl.cur_sec = 0;
	player_ctl.fp = (FIL *)malloc(sizeof(FIL));
	player_ctl.header = (wavhead_t *)malloc(sizeof(wavhead_t));
	player_ctl.buffer0 = (uint8_t *)malloc(I2S_DAM_BUFFER_SIZE);
	player_ctl.buffer1 = (uint8_t *)malloc(I2S_DAM_BUFFER_SIZE);
	
	if (player_ctl.fp == NULL || player_ctl.buffer0 == NULL || 
			player_ctl.buffer1 == NULL || player_ctl.header == NULL) {
		err("���ֲ��ſ�������ʼ��ʧ�ܣ�\r\n");
		return -1;
	}
	
	/* ����WM8978Ϊ����ģʽ */	
	wm->ioctl(wm, WM8978_CMD_SET_PLAYER_MODE, NULL);
	i2s_set_tx_mode(I2S_DataFormat_16b);													/* I2S ����ΪTXģʽ */  //��Ӧ����I2S_DAM_BUFFER_SIZE�Ŷ԰�
	i2s_dma_set_tx_mode(player_ctl.buffer0, player_ctl.buffer1, I2S_DAM_BUFFER_SIZE/2);		/* DMA����ΪTXģʽ */
	i2s_dma_tx_callback = play_audio_i2s_dma_tx_callback;									/* TX DMA���ûص����� */
	
	return 0;
}


/*
 * �뿪¼������ģʽ
*/
int32_t audio_leave_play_mode(void)
{
	free(player_ctl.fp);
	free(player_ctl.header);
	free(player_ctl.buffer0);
	free(player_ctl.buffer1);
	return 0;
}


/*
 * ����ָ��¼���ļ�
 */
int32_t audio_play(char *name)
{
	uint32_t br;
	FRESULT res;
	wavhead_t *wavhead = player_ctl.header;

	if (wavhead_decode(name, player_ctl.header) != 0) {								/* ����WAV�ļ�ͷ */
		err("���� %s ͷ�ļ�ʧ��!\r\n", name);
		return -1;
	}
	
	i2s_set_sample_rate(player_ctl.header->fmt.sample_rate);						/* ���ݽ�������Ϣ���ò����� */
	player_stop();																	/* ��ֹͣ�������� */
	if ((res = f_open(player_ctl.fp, name, FA_READ)) != FR_OK) {
		err("f_open failed!, res = %d\r\n", res);
		return -1;
	}
	
	f_lseek(player_ctl.fp, WAV_HEADER_SIZE);										/* ����WAVͷ�ļ� */
	br = read_file_to_mem(player_ctl.buffer0, I2S_DAM_BUFFER_SIZE);					/* ��ȡ�ļ���DMA����0�� */
	if (br == I2S_DAM_BUFFER_SIZE)
		br = read_file_to_mem(player_ctl.buffer1, I2S_DAM_BUFFER_SIZE);				/* ��ȡ�ļ���DMA����1�� */
	
	player_start();																	/* ��ʼ�������� */
	debug(">> ��ʼ��������\r\n");
	
	player_ctl.tot_sec = wavhead->data.size / wavhead->fmt.byte_rate;				/* ��ʱ�䣺 �����ܴ�С/���� */
	
	while (1) {
		if (player_ctl.transfered_flag == 1)										/* DMA������� */
		{
			player_ctl.transfered_flag = 0;											/* ���DMA������ɱ�־ */
			if (br != I2S_DAM_BUFFER_SIZE)											/* ���Ž����� */
				break;
			
			if (player_ctl.buffer_flag)												/* �ж�Ҫ����Ǹ�buffer? */
				br = read_file_to_mem(player_ctl.buffer1, I2S_DAM_BUFFER_SIZE); 	/* ���buffer1 */
			else
				br = read_file_to_mem(player_ctl.buffer0, I2S_DAM_BUFFER_SIZE);	/* ���buffer0 */
			
			wav_get_cur_time();									/* ��ȡ��ǰ���ŵ���ʱ�� */
			print_audio_msg();
		}
		rt_thread_delay(4);
	}
	
	player_stop();
	debug("|| ֹͣ��������\r\n");
	return 0;
}	


void wav_get_cur_time(void)
{
	long long fops;
	fops = player_ctl.fp->fptr - WAV_HEADER_SIZE;
	player_ctl.cur_sec = fops * player_ctl.tot_sec / player_ctl.header->data.size;
}

void print_audio_msg(void)
{
	debug("��ʱ��: %2d:%2d, ��ǰʱ��: %2d:%2d, ����: %d KByte/S\r", 
			player_ctl.tot_sec/60, player_ctl.tot_sec%60, player_ctl.cur_sec/60,
				player_ctl.cur_sec%60, player_ctl.header->fmt.byte_rate/1000);
	
}

/* wav����ʱ��I2S DMA����ص����� */
void play_audio_i2s_dma_tx_callback(void)
{
	uint16_t i;
	
	if (DMA1_Stream4->CR & (1 << 19)) {
		player_ctl.buffer_flag = 0;
		if ((player_ctl.state & 0x01) == 0) {
			for (i = 0; i < I2S_DAM_BUFFER_SIZE; i++) /* ��ͣ���� */
				player_ctl.buffer0[i] = 0;
		}
	} else {
		player_ctl.buffer_flag = 1;
		if ((player_ctl.state & 0x01) == 0) {
			for (i = 0; i < I2S_DAM_BUFFER_SIZE; i++) /* ��ͣ */
				player_ctl.buffer1[i] = 0;
		}
	}
	
	player_ctl.transfered_flag = 1;	
}

/* 
 * ��ȡ�ļ���ָ�����ڴ��У�����ļ������ڴ���ֱ�����0
��*  buf:���ڴ��ַ����size: Ҫ��ȡ���ֽ���
 */
int32_t read_file_to_mem(uint8_t *buf, uint32_t size)
{
	uint32_t br;							/* �������ֽ��� */
	uint32_t i;
	
	f_read(player_ctl.fp, buf, size, &br); 	/* 16bit��Ƶֱ�Ӷ�ȡ���� */
	if (br < size) {
		for (i = br; i < size - br; i++)	/* ��������ʱ�����0 */
			buf[i] = 0;
	}
	
	return br;
}	

/* 
 * ¼���� I2S_DMA�����жϷ����������ж����潫DMA���������д���ļ� 
 */
extern rt_event_t fopevent;
void record_audio_i2s_dma_rx_callback(void)
{
	if (recorder_ctl.state & (1 << 7)) { 															/* �ѿ���¼�� */														
		if (DMA1_Stream3->CR & (1 << 19)) {															/* ��ǰĿ��洢��Ϊbuffer0 */
			event_send(fopevent, AUDIO_EVENT_0);																						
		} else	{																					/* ��ǰĿ��洢��Ϊbuffer1 */													
			event_send(fopevent, AUDIO_EVENT_1);
		}
	}
}

//�������жϵķ�ʽд��
int32_t audio_set_record_mode(wm8978_t *wm)
{
	/* ��ʼ��¼���������� */
	recorder_ctl.wm = wm;
	recorder_ctl.state = 0;
	recorder_ctl.data_size = 0;
	recorder_ctl.fp = (FIL *)malloc(sizeof(FIL));
	recorder_ctl.header = (wavhead_t *)malloc(sizeof(wavhead_t));
	recorder_ctl.buffer0 = (uint8_t *)malloc(I2S_DAM_BUFFER_SIZE);
	recorder_ctl.buffer1 = (uint8_t *)malloc(I2S_DAM_BUFFER_SIZE);
	assert(recorder_ctl.fp);	//����Ƿ����ռ�
	assert(recorder_ctl.buffer0);
	assert(recorder_ctl.buffer1);
	assert(recorder_ctl.header);
	wavhead_encode(recorder_ctl.header);
//	wm->ioctl(wm, WM8978_CMD_SET_RECORDER_MODE, NULL);		/* ����WM8978Ϊ¼��ģʽ */
	/*
	I2S2_Init
	I2S2ext_Init
	I2S2_TX_DMA_Init
	I2S2ext_RX_DMA_Init
	*/
	/*
	 * ������WM8978�ṩWS�� CK��MCK��ʱ�ӣ���ʱ��Ҫ¼��������ֻ��ʹ��ȫ˫������I2S2��������0x0000��WM8978
	 * �Բ���CK��WS,��MCK���źţ���I2S2_ext���������WM8978��ADC���ݣ�I2S2ext_SD��, �����浽SD����ʵ��¼��
	 */
	i2s_set_tx_mode(I2S_DataFormat_16b);	/* I2S2 ����Ϊ��������ģʽ */
	i2s_set_rx_mode(I2S_DataFormat_16b);	/* I2S2_ext����Ϊ�ӻ�����ģʽ */

	i2s_set_sample_rate(AUDIO_SAMPLE_RATE);				/* ���ò�����16000 */
	i2s_dma_set_tx_mode((uint8_t *)&i2s_playbuf[0], (uint8_t *)&i2s_playbuf[1], 1);
	DMA1_Stream4->CR &= ~(1 << 4); 			/* ¼��ʱ����Ҫ����Stream4 DMA�����ж� */
	
	i2s_dma_set_rx_mode(recorder_ctl.buffer0, recorder_ctl.buffer1, I2S_DAM_BUFFER_SIZE/2);
	i2s_dma_rx_callback = record_audio_i2s_dma_rx_callback;	
	
	DMA_Cmd(DMA1_Stream4, ENABLE);			/* ����DMA TX����, ���ǲ�����Ҫ��������ж� */

	return 0;
}

/* �ͷ�¼��������������ڴ� */
int32_t audio_leave_record_mode(void)
{
	free(recorder_ctl.fp);
	free(recorder_ctl.header);
	free(recorder_ctl.buffer0);
	free(recorder_ctl.buffer1);
	return 0;
}


/*
 * ¼���� ָ���ļ�����¼��ʱ��
*/
int32_t audio_record(char *name, uint32_t sec)
{
	uint32_t bw;
	FRESULT res;
	
	if ((res = f_open(recorder_ctl.fp, name, FA_OPEN_ALWAYS | FA_WRITE)) != FR_OK) {
		err("��/���� %s ����! res: %d\r\n", name, res);
		return -1;
	}
	
	/* ��wavͷд������� */
	if((res = f_write(recorder_ctl.fp, recorder_ctl.header, sizeof(wavhead_t), &bw)) != FR_OK) {
		err("wavhead д����̳���\r\n res: %d", res);
		return -1;
	}
	
	recorder_start();						/* ��ʼ¼�� */

	debug(">> ��ʼ¼�� %d��\r\n", sec);
	
	sleep(sec);								/* ¼��ʱ�� */
	
	recorder_stop();						/* ֹͣ¼�� */
	debug("|| ����¼�� %d��\r\n", sec);
	
	debug("¼�����ݴ�С: %d\n", recorder_ctl.data_size);
	wavhead_write_data_size(recorder_ctl.fp, recorder_ctl.header, recorder_ctl.data_size);
	
	f_close(recorder_ctl.fp);
	
	return 0;
}


/* ¼�������� */
void recorder_start(void)
{
	recorder_ctl.state |= 1 << 7;
	i2s_dma_rx_start();
}

/* ¼����ֹͣ */
void recorder_stop(void)
{
	recorder_ctl.state &= 0 << 7;
	i2s_dma_rx_stop();
}

/* ��������ʼ */
void player_start(void)
{
	player_ctl.state = 3 << 0;
	i2s_dma_tx_start();
}

/* ������ֹͣ */
void player_stop(void)
{
	player_ctl.state = 0;
	i2s_dma_tx_stop();
}


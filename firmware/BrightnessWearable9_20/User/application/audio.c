#include "audio.h"
#include "i2s.h"
#include "dma.h"
#include "config.h"
#include "fatfserr.h"



const uint16_t i2s_playbuf[2] = {0xdead, 0xbeef};		/* 2个16位数据,用于录音时I2S Master发送.循环发送0. */

volatile player_t 			player_ctl;				/* 播放器控制块 */
volatile recorder_t 			recorder_ctl;			/* 播放器控制块 */


void recorder_start(void);
void recorder_stop(void);
void player_start(void);						/* 启动播放器 */
void player_stop(void);						/* 停止播放器 */
void wav_get_cur_time(void);
void play_audio_i2s_dma_tx_callback(void);		/* I2S DMA TX 回调函数 */
int32_t read_file_to_mem(uint8_t *buf, uint32_t size);
void print_audio_msg(void);

/* 
 * 音乐播放模式
 */
int32_t audio_set_play_mode(wm8978_t *wm)
{
	
	if (wm == NULL) {
		err("wm == NULL\r\n");
		return -1;
	}
	
	/* 初始化音乐播放控制器 */
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
		err("音乐播放控制器初始化失败！\r\n");
		return -1;
	}
	
	/* 设置WM8978为播放模式 */	
	wm->ioctl(wm, WM8978_CMD_SET_PLAYER_MODE, NULL);
	i2s_set_tx_mode(I2S_DataFormat_16b);													/* I2S 设置为TX模式 */  //这应该是I2S_DAM_BUFFER_SIZE才对吧
	i2s_dma_set_tx_mode(player_ctl.buffer0, player_ctl.buffer1, I2S_DAM_BUFFER_SIZE/2);		/* DMA设置为TX模式 */
	i2s_dma_tx_callback = play_audio_i2s_dma_tx_callback;									/* TX DMA设置回调函数 */
	
	return 0;
}


/*
 * 离开录音播放模式
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
 * 播放指定录音文件
 */
int32_t audio_play(char *name)
{
	uint32_t br;
	FRESULT res;
	wavhead_t *wavhead = player_ctl.header;

	if (wavhead_decode(name, player_ctl.header) != 0) {								/* 解析WAV文件头 */
		err("解析 %s 头文件失败!\r\n", name);
		return -1;
	}
	
	i2s_set_sample_rate(player_ctl.header->fmt.sample_rate);						/* 根据解析的信息设置采样率 */
	player_stop();																	/* 先停止播放音乐 */
	if ((res = f_open(player_ctl.fp, name, FA_READ)) != FR_OK) {
		err("f_open failed!, res = %d\r\n", res);
		return -1;
	}
	
	f_lseek(player_ctl.fp, WAV_HEADER_SIZE);										/* 跳过WAV头文件 */
	br = read_file_to_mem(player_ctl.buffer0, I2S_DAM_BUFFER_SIZE);					/* 读取文件到DMA缓存0中 */
	if (br == I2S_DAM_BUFFER_SIZE)
		br = read_file_to_mem(player_ctl.buffer1, I2S_DAM_BUFFER_SIZE);				/* 读取文件到DMA缓存1中 */
	
	player_start();																	/* 开始播放音乐 */
	debug(">> 开始播放音乐\r\n");
	
	player_ctl.tot_sec = wavhead->data.size / wavhead->fmt.byte_rate;				/* 总时间： 数据总大小/码率 */
	
	while (1) {
		if (player_ctl.transfered_flag == 1)										/* DMA传输完成 */
		{
			player_ctl.transfered_flag = 0;											/* 清楚DMA传输完成标志 */
			if (br != I2S_DAM_BUFFER_SIZE)											/* 播放结束？ */
				break;
			
			if (player_ctl.buffer_flag)												/* 判断要填充那个buffer? */
				br = read_file_to_mem(player_ctl.buffer1, I2S_DAM_BUFFER_SIZE); 	/* 填充buffer1 */
			else
				br = read_file_to_mem(player_ctl.buffer0, I2S_DAM_BUFFER_SIZE);	/* 填充buffer0 */
			
			wav_get_cur_time();									/* 获取当前播放到的时刻 */
			print_audio_msg();
		}
		rt_thread_delay(4);
	}
	
	player_stop();
	debug("|| 停止播放音乐\r\n");
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
	debug("总时间: %2d:%2d, 当前时间: %2d:%2d, 码率: %d KByte/S\r", 
			player_ctl.tot_sec/60, player_ctl.tot_sec%60, player_ctl.cur_sec/60,
				player_ctl.cur_sec%60, player_ctl.header->fmt.byte_rate/1000);
	
}

/* wav播放时，I2S DMA传输回调函数 */
void play_audio_i2s_dma_tx_callback(void)
{
	uint16_t i;
	
	if (DMA1_Stream4->CR & (1 << 19)) {
		player_ctl.buffer_flag = 0;
		if ((player_ctl.state & 0x01) == 0) {
			for (i = 0; i < I2S_DAM_BUFFER_SIZE; i++) /* 暂停播放 */
				player_ctl.buffer0[i] = 0;
		}
	} else {
		player_ctl.buffer_flag = 1;
		if ((player_ctl.state & 0x01) == 0) {
			for (i = 0; i < I2S_DAM_BUFFER_SIZE; i++) /* 暂停 */
				player_ctl.buffer1[i] = 0;
		}
	}
	
	player_ctl.transfered_flag = 1;	
}

/* 
 * 读取文件到指定的内存中，如果文件不够内存中直接填充0
　*  buf:　内存地址，　size: 要读取的字节数
 */
int32_t read_file_to_mem(uint8_t *buf, uint32_t size)
{
	uint32_t br;							/* 读到的字节数 */
	uint32_t i;
	
	f_read(player_ctl.fp, buf, size, &br); 	/* 16bit音频直接读取数据 */
	if (br < size) {
		for (i = br; i < size - br; i++)	/* 不完整的时候填充0 */
			buf[i] = 0;
	}
	
	return br;
}	

/* 
 * 录音， I2S_DMA接受中断服务函数，在中断里面将DMA缓冲的数据写入文件 
 */
extern rt_event_t fopevent;
void record_audio_i2s_dma_rx_callback(void)
{
	if (recorder_ctl.state & (1 << 7)) { 															/* 已开启录音 */														
		if (DMA1_Stream3->CR & (1 << 19)) {															/* 当前目标存储器为buffer0 */
			event_send(fopevent, AUDIO_EVENT_0);																						
		} else	{																					/* 当前目标存储器为buffer1 */													
			event_send(fopevent, AUDIO_EVENT_1);
		}
	}
}

//数据以中断的方式写入
int32_t audio_set_record_mode(wm8978_t *wm)
{
	/* 初始化录音器控制器 */
	recorder_ctl.wm = wm;
	recorder_ctl.state = 0;
	recorder_ctl.data_size = 0;
	recorder_ctl.fp = (FIL *)malloc(sizeof(FIL));
	recorder_ctl.header = (wavhead_t *)malloc(sizeof(wavhead_t));
	recorder_ctl.buffer0 = (uint8_t *)malloc(I2S_DAM_BUFFER_SIZE);
	recorder_ctl.buffer1 = (uint8_t *)malloc(I2S_DAM_BUFFER_SIZE);
	assert(recorder_ctl.fp);	//检查是否分配空间
	assert(recorder_ctl.buffer0);
	assert(recorder_ctl.buffer1);
	assert(recorder_ctl.header);
	wavhead_encode(recorder_ctl.header);
//	wm->ioctl(wm, WM8978_CMD_SET_RECORDER_MODE, NULL);		/* 设置WM8978为录音模式 */
	/*
	I2S2_Init
	I2S2ext_Init
	I2S2_TX_DMA_Init
	I2S2ext_RX_DMA_Init
	*/
	/*
	 * 必须向WM8978提供WS， CK和MCK等时钟，用时又要录音，所以只能使用全双工。主I2S2发送数据0x0000给WM8978
	 * 以产生CK，WS,和MCK等信号，从I2S2_ext则接受来自WM8978的ADC数据（I2S2ext_SD）, 并保存到SD卡，实现录音
	 */
	i2s_set_tx_mode(I2S_DataFormat_16b);	/* I2S2 配置为主机发送模式 */
	i2s_set_rx_mode(I2S_DataFormat_16b);	/* I2S2_ext配置为从机接受模式 */

	i2s_set_sample_rate(AUDIO_SAMPLE_RATE);				/* 设置采样率16000 */
	i2s_dma_set_tx_mode((uint8_t *)&i2s_playbuf[0], (uint8_t *)&i2s_playbuf[1], 1);
	DMA1_Stream4->CR &= ~(1 << 4); 			/* 录音时不需要开启Stream4 DMA传输中断 */
	
	i2s_dma_set_rx_mode(recorder_ctl.buffer0, recorder_ctl.buffer1, I2S_DAM_BUFFER_SIZE/2);
	i2s_dma_rx_callback = record_audio_i2s_dma_rx_callback;	
	
	DMA_Cmd(DMA1_Stream4, ENABLE);			/* 开启DMA TX传输, 但是并不需要传输完成中断 */

	return 0;
}

/* 释放录音过程中申请的内存 */
int32_t audio_leave_record_mode(void)
{
	free(recorder_ctl.fp);
	free(recorder_ctl.header);
	free(recorder_ctl.buffer0);
	free(recorder_ctl.buffer1);
	return 0;
}


/*
 * 录音， 指定文件名和录音时间
*/
int32_t audio_record(char *name, uint32_t sec)
{
	uint32_t bw;
	FRESULT res;
	
	if ((res = f_open(recorder_ctl.fp, name, FA_OPEN_ALWAYS | FA_WRITE)) != FR_OK) {
		err("打开/创建 %s 出错! res: %d\r\n", name, res);
		return -1;
	}
	
	/* 把wav头写入磁盘中 */
	if((res = f_write(recorder_ctl.fp, recorder_ctl.header, sizeof(wavhead_t), &bw)) != FR_OK) {
		err("wavhead 写入磁盘出错！\r\n res: %d", res);
		return -1;
	}
	
	recorder_start();						/* 开始录音 */

	debug(">> 开始录音 %d秒\r\n", sec);
	
	sleep(sec);								/* 录音时长 */
	
	recorder_stop();						/* 停止录音 */
	debug("|| 结束录音 %d秒\r\n", sec);
	
	debug("录音数据大小: %d\n", recorder_ctl.data_size);
	wavhead_write_data_size(recorder_ctl.fp, recorder_ctl.header, recorder_ctl.data_size);
	
	f_close(recorder_ctl.fp);
	
	return 0;
}


/* 录音器开启 */
void recorder_start(void)
{
	recorder_ctl.state |= 1 << 7;
	i2s_dma_rx_start();
}

/* 录音器停止 */
void recorder_stop(void)
{
	recorder_ctl.state &= 0 << 7;
	i2s_dma_rx_stop();
}

/* 播放器开始 */
void player_start(void)
{
	player_ctl.state = 3 << 0;
	i2s_dma_tx_start();
}

/* 播放器停止 */
void player_stop(void)
{
	player_ctl.state = 0;
	i2s_dma_tx_stop();
}


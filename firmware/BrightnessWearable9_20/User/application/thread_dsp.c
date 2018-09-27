#include "thread_dsp.h"
#include "arm_math.h"
#include "arm_const_structs.h"
#include "fatfserr.h"
#include "config.h"
#include "audio.h"
#include "window.h"
#include "feature.h"

#ifdef THREAD
#undef THREAD
#endif
#define THREAD 		"[Thread_DSP]"

extern rt_event_t audio_feature;
extern rt_event_t fopevent;
extern mutex_flag_t stop_key_dsp;
extern struct dsp_buffer dspdatabuf;
extern struct name_buffer old_name_wav;
extern struct fft_buffer in_dsp_databuf;

uint8_t 		dsp_stop_flag = 0;

static void feature_init_computing(void);
static void feature_start_computing(void);
static void feature_computing(struct fft_buffer *in_dsp_buffer);
static void feature_continue_computing(void);
static void feature_stop_computing(void);
static void check_need_change(void);

void rt_dsp_thread_entry(void *parameter)
{
	rt_uint32_t 	revent;
	struct fft_buffer 	*in_dsp_buffer = &in_dsp_databuf;
	
	while(1) {
		event_recv(audio_feature, (AUDIO_STOP_CMPL | DSP_START_CMPL | DSP_STOP_CMPL | 
		           DSP_READ_EVENT_CMPL | DSP_WRITE_EVENT_CMPL | DSP_READY_STOP), 
				   RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, &revent);
		if (revent & AUDIO_STOP_CMPL) {
			feature_init_computing();			
		}
		if(!dsp_stop_flag && !(revent & DSP_STOP_CMPL)){							//应该处理未处理的音频文件
			if (revent & DSP_START_CMPL) {
				feature_start_computing();
			}
			if (revent & DSP_READ_EVENT_CMPL) {
				feature_computing(in_dsp_buffer);
			}
			if (revent & DSP_WRITE_EVENT_CMPL) {
				feature_continue_computing();
			}
			if(stop_key_dsp.flag){
				old_name_wav.using_flag = 1;
			}
            if (revent & DSP_READY_STOP) {
                event_send(fopevent, DSP_STOP);;
            }
		}
        if (revent & DSP_STOP_CMPL) {
			feature_stop_computing();			
		}
	}
}

static void feature_init_computing(void)
{
	debug(THREAD "接收到开始处理音频事件!\r\n");
	dsp_stop_flag = 0;
	
	if(!stop_key_dsp.flag){
		debug(THREAD "feature_init_computing知道停止按键没有按下！\n");
		rt_memcpy(old_name_wav.name_buffer2, old_name_wav.name_buffer1,
		rt_strlen(old_name_wav.name_buffer1));
		debug(THREAD "发送创建DSP处理文件事件~\r\n");
		event_send(fopevent, DSP_START);
	} else {
		debug(THREAD "feature_init_computing停止按键已经按下！\n");
		if(old_name_wav.using_flag == 0){
			rt_memcpy(old_name_wav.name_buffer2, old_name_wav.name_buffer1,
			rt_strlen(old_name_wav.name_buffer1));
			debug(THREAD "发送创建DSP处理文件事件~\r\n");
			event_send(fopevent, DSP_START);
		}
//		debug("using_flag:%d\n", old_name_wav.using_flag);
	}	
//	char  FFT_test_file[FILE_NAME_LENGTH] = "AUDIO_FFT_TEST.wav";
//	rt_memcpy(old_name_wav.name_buffer2, FFT_test_file,
//	rt_strlen(FFT_test_file));
}

static void feature_start_computing(void)
{
	debug(THREAD "接收到开始DSP处理事件!\r\n");
	
	uint32_t window_length = old_name_wav.second/1024;
	debug("window_number:%d\n", window_length);	
	
	debug(THREAD "发送读取已有语音文件事件~\r\n");
	event_send(fopevent, DSP_READ_EVENT);
}
static void feature_computing(struct fft_buffer *in_dsp_buffer)
{
	uint32_t out_formant[5];
	float 	 in_formant[256];
//	debug(THREAD "把整形转化浮点输入~\r\n");
	for(int i = 0, j = 0; j < WIN_LEN_SIZE; j++, i = i + 2){
		in_dsp_buffer->energy_buffer_voice[j] = (float)dspdatabuf.buffer[i]/32768;
		in_dsp_buffer->entropy_buffer_voice[j] = in_dsp_buffer->energy_buffer_voice[j];
		in_dsp_buffer->formant_buffer_voice[j] = in_dsp_buffer->energy_buffer_voice[j];
	}
	
	int32_t result_energy = (int)(get_energy_window(in_dsp_buffer->energy_buffer_voice)*100);
	//get_entropy_brightness_window函数同时计算亮度和谱熵两个音频特征
	int32_t result_brightness = 0;
	int32_t result_entropy = (int)(get_entropy_brightness_window(in_dsp_buffer->entropy_buffer_voice, &result_brightness)*10000);
	get_formant_window(in_dsp_buffer->formant_buffer_voice, in_formant, out_formant);
	
	rt_memset(dspdatabuf.feature_buffer, 0, 72);
	rt_snprintf(dspdatabuf.feature_buffer, 72, "%7d, %7d, %7d, %7d, %7d, %7d, %7d, %7d\n", 
			   result_energy, result_entropy, result_brightness, out_formant[0], out_formant[1],
			   out_formant[2], out_formant[3], out_formant[4]);
	
//	debug(THREAD "发送写入语音特征事件~\r\n");
	event_send(fopevent, DSP_WRITE_EVENT);	
}

static void feature_continue_computing(void)
{
//	debug("~~~~~~~~~~~~~~继续读数据~~~~~~~~~~~~~~\r\n");
//	debug(THREAD "继续读数据\n");
	event_send(fopevent, DSP_READ_EVENT);
}
static void check_need_change(void)
{
	uint8_t length = rt_strlen(old_name_wav.name_buffer1);
	
	old_name_wav.using_flag = rt_memcmp(old_name_wav.name_buffer1, old_name_wav.name_buffer2, length);
}

static void feature_stop_computing(void)
{
	debug(THREAD "特征提取成功！\n");
	if(!stop_key_dsp.flag){
        dsp_stop_flag = 1;						//将等待新的语音文件
		debug(THREAD "feature_stop_computing知道停止按键没有按下！\n");
		old_name_wav.using_flag = 0;
	} else {
		debug(THREAD "feature_stop_computing停止按键已经按下！\n");
		check_need_change();
		debug(THREAD "name_buffer1:%s\n", old_name_wav.name_buffer1);
		debug(THREAD "name_buffer2:%s\n", old_name_wav.name_buffer2);
		debug(THREAD"using_flag:%d\n", old_name_wav.using_flag);
		if(old_name_wav.using_flag == 1){  //按键按下后还有文件没有处理完
			old_name_wav.using_flag = 0;
			feature_init_computing();      //再处理buffer1里面的文件
		} else {
            dsp_stop_flag = 1;						//最后停止处理音频文件
            debug(THREAD"~~最后停止处理音频文件！~~\n", old_name_wav.using_flag);
        }			
	}
}





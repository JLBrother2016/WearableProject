#include "thread_file.h"
#include "config.h"
#include "fatfserr.h"
#include "rtc.h"
#include "audio.h"

#ifdef THREAD
#undef THREAD
#endif
#define THREAD 		"[Thread_File]"

extern volatile recflg_t flg;
extern rt_event_t fopevent;
extern rt_event_t fopcmpl;
extern rt_event_t audio_feature;
extern struct mpu_buffer mpubuffer;
extern struct sensors_buffer sensorsbuf;
extern struct dsp_buffer dspdatabuf;
extern struct name_buffer old_name_wav;

static void do_mpu_start(FIL *fp, char *path, uint8_t length);
static void do_mpu_event(FIL *fp, uint32_t *wb);
static void do_mpu_stop(FIL *fp, char *fname, uint8_t length);
static void do_sensors_start(FIL *fp, char *fname, uint8_t length);
static void do_sensors_event(FIL *fp, uint32_t *wb);
static void do_sensors_stop(FIL *fp, char *fname, uint8_t length);
static void do_audio_start(FIL *fp, char *fname, uint8_t length);
static void do_audio_event_0(FIL *fp, uint32_t *wb);
static void do_audio_event_1(FIL *fp, uint32_t *wb);
static void do_audio_stop(FIL *fp, char *fname, uint8_t length);
static void do_dsp_start(FIL *f_lastp, FIL *fp, char *fname);
static void do_dsp_read_event(FIL *fp, uint32_t *rb);
static void do_dsp_write_event(FIL *fp, uint32_t *wb);
static void do_dsp_stop(FIL *fp, FIL *f_lastp, char *fname);

typedef enum 
{
	MPU_TYPE 			= 0,
	SENSORS_TYPE		= 1,
	AUDIO_TYPE			= 2,
	DSP_TYPE 			= 3
} file_type_t;

wavhead_t			last_audio_head;

static void format_name(file_type_t type, char *name, uint8_t length)
{
	rtc_t 		rtc;				//RTC
	get_rtc_info(&rtc);
	
	switch (type) {
		case MPU_TYPE:
			rt_snprintf(name, length, "MPU_%04d-%02d-%02d-%02d-%02d-%02d.csv", 
					rtc.year, 
					rtc.month, 
					rtc.day, 
					rtc.hour, 
					rtc.min, 
					rtc.sec);
			break;
		case SENSORS_TYPE:
			rt_snprintf(name, length, "SENSORS_%04d-%02d-%02d-%02d-%02d-%02d.csv", 
					rtc.year, 
					rtc.month, 
					rtc.day, 
					rtc.hour, 
					rtc.min, 
					rtc.sec);
			break;
		
		case AUDIO_TYPE:
			rt_snprintf(name, length, "AUDIO_%04d-%02d-%02d-%02d-%02d-%02d.wav", 
					rtc.year, 
					rtc.month, 
					rtc.day, 
					rtc.hour, 
					rtc.min, 
					rtc.sec);
			break;
		case DSP_TYPE:
			rt_snprintf(name, length, "AUDIO_%04d-%02d-%02d-%02d-%02d-%02d.csv", 
					rtc.year, 
					rtc.month, 
					rtc.day, 
					rtc.hour, 
					rtc.min, 
					rtc.sec);
			break;
		
		default:
			err("format_name error! No such type: %d\r\n", type);
	}
}

//�ļ������̣߳�ר�����������ļ�ϵͳ�������̲߳���ֱ��
//�����ļ�ϵͳ��ֻ�ܷ����¼������ļ������߳���������
void rt_file_thread_entry(void *parameter)
{
	FIL 				fp_mpu;
	FIL					fp_sensors;
	FIL					fp_audio;
	FIL					fp_dsp;
	FIL					fp_last_audio;
	
	char 				mpu_fname[FILE_NAME_LENGTH];
	char 				sensors_fname[FILE_NAME_LENGTH];
	char 				audio_fname[FILE_NAME_LENGTH];
	char 				dsp_fname[FILE_NAME_LENGTH];
	
	uint32_t 			wb;					//д���ļ��ֽ���
	rt_uint32_t 		revent;				//���ص��¼�
	
	uint32_t 			size_mpu = 0;		//д��MPU�����ֽ���
	uint32_t 			size_sensors = 0;	//д��SENSORS�����ֽ���
	uint32_t 			size_audio = 0;		//д��AUDIO�����ֽ���
	uint32_t 			size_last = 0;		//д��DSP����������ֽ���
	uint32_t 			size_feature = 0;	//д��DSP����������ֽ���
	uint32_t 			window_nubmber = 0;
	
	while (1) {
		event_recv(fopevent, ( MPU_START | MPU_EVENT | MPU_STOP | 
				   AUDIO_START | AUDIO_EVENT_0 | AUDIO_EVENT_1 | AUDIO_STOP |
				   SENSORS_START | SENSORS_EVENT | SENSORS_STOP | DSP_START | 
				   DSP_WRITE_EVENT | DSP_READ_EVENT | DSP_STOP), 
				   RT_EVENT_FLAG_OR | RT_EVENT_FLAG_CLEAR, &revent);
		
		//============================================================================
		if (revent & MPU_START) { 		//����mpu�ļ�
			size_mpu = 0;
			do_mpu_start(&fp_mpu, mpu_fname, sizeof(mpu_fname));
		}
		
		if (revent & MPU_EVENT) { 		//дmpu����
			do_mpu_event(&fp_mpu, &wb);
			size_mpu += wb;
//			debug(THREAD "MPU���ݴ�С�� %dKB\n", size_mpu/1024);
		}
		
		if (revent & MPU_STOP) {		//ֹͣmpuд�ļ�
			do_mpu_stop(&fp_mpu, mpu_fname, sizeof(mpu_fname));
		}
		
		
		//=================================================================================================================================
		if (revent & SENSORS_START) {	//����sensors�ļ�
			size_sensors = 0;
			do_sensors_start(&fp_sensors, sensors_fname, sizeof(sensors_fname));
		}
		
		if (revent & SENSORS_EVENT) {	//дsensors����	
			do_sensors_event(&fp_sensors, &wb);
			size_mpu += wb;
//			debug(THREAD "SENSORS���ݴ�С�� %dKB\n", size_sensors/1024);
		}
			
		if (revent & SENSORS_STOP) { 	//ֹͣsensorд�ļ�			
			do_sensors_stop(&fp_sensors, sensors_fname, sizeof(sensors_fname));
		}
		
		//========================================================================================
		if (revent & AUDIO_START) {		//����audio�ļ�
			
			size_audio = 0;	
			do_audio_start(&fp_audio, audio_fname, sizeof(audio_fname));
			recorder_start();						// ��ʼ¼��
		}
		
		if (revent & AUDIO_EVENT_0) { //д��Ƶbuffer0
			
			do_audio_event_0(&fp_audio, &wb);
			size_audio += wb;
//			debug(THREAD "AUDIO���ݴ�С�� %dKB\n", size_audio/1024);

		}
		
		if (revent & AUDIO_EVENT_1) {//д��Ƶbuffer1
			
			do_audio_event_1(&fp_audio, &wb);
			size_audio += wb;
//			debug(THREAD "AUDIO���ݴ�С�� %dKB\n", size_audio/1024);
		}
		
		if (revent & AUDIO_STOP) {//ֹͣaudio�ļ�д
			do_audio_stop(&fp_audio, audio_fname, sizeof(sensors_fname));
		}
		
		//============================================================================
		if (revent & DSP_START) { 		//����dsp�ļ�
			size_feature = 0;
			do_dsp_start(&fp_last_audio, &fp_dsp, dsp_fname);
			window_nubmber = old_name_wav.second / 1024;
		}
		
		if (revent & DSP_READ_EVENT) { 		//��ȡ��Ƶ����
			do_dsp_read_event(&fp_last_audio, &wb);
			size_last += wb;
//			debug(THREAD "��ȡ���ݴ�С�� %dKB\n", size_last/1024);
		}
		
		if(window_nubmber != 0 && size_last/1024 == window_nubmber){
			event_send(audio_feature, DSP_READY_STOP);
		}
		
		if (revent & DSP_WRITE_EVENT) { 		//дfeature����
			do_dsp_write_event(&fp_dsp, &wb);
			size_feature += wb;
//			debug(THREAD "DSPд�����ݴ�С�� %dKB\n", size_feature/1024);
		}
		
		if (revent & DSP_STOP) {		//ֹͣdspд�ļ�
			size_last = 0;
			size_last = 0;
			window_nubmber = 0;
			do_dsp_stop(&fp_dsp, &fp_last_audio, dsp_fname);			
		}
		
	}
}

static void do_mpu_start(FIL *fp, char *fname, uint8_t length)
{
	rt_memset(fname, 0, length);
	format_name(MPU_TYPE, fname, length);
	open(fp, fname, FA_OPEN_ALWAYS | FA_WRITE);
	debug(THREAD "%s�ļ������ɹ���\r\n", fname);
	event_send(fopcmpl, MPU_START_CMPL);
	debug(THREAD "����MPU�ļ���������¼�~\r\n");
}

static void do_mpu_event(FIL *fp, uint32_t *wb)
{	
	if (mpubuffer.full == 1) {	
		mutex_lock(mpubuffer.mutex);
		write(fp, mpubuffer.buffer, mpubuffer.used, wb);
		mpubuffer.full = 0; //buffer������д��SD��
		mpubuffer.used = 0;
		rt_memset(mpubuffer.buffer, 0, MPU_BUFFER_SIZE);
		mutex_unlock(mpubuffer.mutex);

	} else {
		err("error, mpubufferδ��!!\r\n");
		while (1);
	}

	event_send(fopcmpl, MPU_EVENT_CMPL);
//	debug(THREAD "����MPU�ļ���������¼�~\r\n");
}
	
static void do_mpu_stop(FIL *fp, char *fname, uint8_t length)
{
	char 		stop_name[128];
	char 		old_name[128];
	uint32_t  	wb;
	
	if (mpubuffer.used != 0) { //������������û��д�����
		write(fp, mpubuffer.buffer, mpubuffer.used, &wb); 
		mpubuffer.full = 0; 	//buffer������д��SD��
		mpubuffer.used = 0;
		rt_memset(mpubuffer.buffer, 0, MPU_BUFFER_SIZE);
	}
	close(fp);
	debug(THREAD "�ļ�%s�رճɹ�~\r\n", fname);
	
	format_name(MPU_TYPE, stop_name, 128);
	rt_memcpy(old_name, fname, length);
	rt_memcpy(fname+rt_strlen(fname)-4, stop_name+3, rt_strlen(stop_name)-3);
	
	rename(old_name, fname);
	debug(THREAD "�������ļ�, ���ļ���: %s\r\n", fname);
	event_send(fopcmpl, MPU_STOP_CMPL);
	debug(THREAD "���͹ر�MPU�ļ���������¼�~\r\n");
}


static void do_sensors_start(FIL *fp, char *fname, uint8_t length)
{
	rt_memset(fname, 0, length);
	format_name(SENSORS_TYPE, fname, length);
	open(fp, fname, FA_OPEN_ALWAYS | FA_WRITE);
	debug(THREAD "%s�ļ������ɹ���\r\n", fname);
	event_send(fopcmpl, SENSORS_START_CMPL);
	debug(THREAD "����SENSORS�ļ���������¼�~\r\n");
}

static void do_sensors_event(FIL *fp, uint32_t *wb)
{
	mutex_lock(sensorsbuf.mutex);
	write(fp, sensorsbuf.buffer, rt_strlen(sensorsbuf.buffer), wb);
	event_send(fopcmpl, SENSORS_EVENT_CMPL);
	mutex_unlock(sensorsbuf.mutex);
//	debug(THREAD "����SENSORS�ļ���������¼�~\r\n");
}

static void do_sensors_stop(FIL *fp, char *fname, uint8_t length)
{
	char 		stop_name[128];
	char 		old_name[128];
	
	close(fp);
	debug(THREAD "�ļ�%s�رճɹ�~\r\n", fname);
	
	format_name(SENSORS_TYPE, stop_name, 128);
	rt_memcpy(old_name, fname, length);
	rt_memcpy(fname+rt_strlen(fname)-4, stop_name+7, rt_strlen(stop_name)-7);
	
	rename(old_name, fname);
	debug(THREAD "�������ļ�, ���ļ���: %s\r\n", fname);
	event_send(fopcmpl, SENSORS_STOP_CMPL);
	debug(THREAD "���͹ر�SENSORS�ļ���������¼�~\r\n");
}

static void do_audio_start(FIL *fp, char *fname, uint8_t length)
{
	uint32_t  	wb;
	
	rt_memset(fname, 0, length);
	format_name(AUDIO_TYPE, fname, length);
	open(fp, fname, FA_OPEN_ALWAYS | FA_WRITE);
	debug(THREAD "%s�ļ������ɹ���\r\n", fname);
	
//	write(recorder_ctl.fp, recorder_ctl.header, sizeof(wavhead_t), &wb);
	write(fp, recorder_ctl.header, sizeof(wavhead_t), &wb);
//	sync(recorder_ctl.fp);
	sync(fp);
	
	debug(THREAD "����AUDIO�ļ���������¼�~\r\n");
	event_send(fopcmpl, AUDIO_START_CMPL);
	
	recorder_start();						// ��ʼ¼��
}

static void do_audio_event_0(FIL *fp, uint32_t *wb)
{
//	write(recorder_ctl.fp, recorder_ctl.buffer0, I2S_DAM_BUFFER_SIZE, wb);
	write(fp, recorder_ctl.buffer0, I2S_DAM_BUFFER_SIZE, wb);
	recorder_ctl.data_size += *wb;
	rt_memset(recorder_ctl.buffer0, 0, I2S_DAM_BUFFER_SIZE);
}

static void do_audio_event_1(FIL *fp, uint32_t *wb)
{
//	write(recorder_ctl.fp, recorder_ctl.buffer1, I2S_DAM_BUFFER_SIZE, wb);
	write(fp, recorder_ctl.buffer1, I2S_DAM_BUFFER_SIZE, wb);
	recorder_ctl.data_size += *wb;
	rt_memset(recorder_ctl.buffer1, 0, I2S_DAM_BUFFER_SIZE);
}

static void do_audio_stop(FIL *fp, char *fname, uint8_t length)
{
	char 		stop_name[128];
	char 		old_name[128];
	
//	wavhead_write_data_size(recorder_ctl.fp, recorder_ctl.header, recorder_ctl.data_size);
	wavhead_write_data_size(fp, recorder_ctl.header, recorder_ctl.data_size);
//	close(recorder_ctl.fp);
	close(fp);
	format_name(AUDIO_TYPE, stop_name, 128);
	
	rt_memcpy(old_name, fname, length);
	rt_memcpy(fname+rt_strlen(fname)-4, stop_name+5, rt_strlen(stop_name)-5);
	
	rename(old_name, fname);
	debug(THREAD "�������ļ�, ���ļ���: %s\r\n", fname);
	audio_leave_record_mode();
	debug(THREAD "���͹ر�AUDIO�ļ���������¼�~\r\n");
	
	rt_memset(old_name_wav.name_buffer1, 0, rt_strlen(fname));
	rt_memcpy(old_name_wav.name_buffer1, fname, rt_strlen(fname));
//    char  FFT_test_file[FILE_NAME_LENGTH] = "AUDIO_FFT_TEST.wav";
//    rt_memcpy(old_name_wav.name_buffer1, FFT_test_file,
//		rt_strlen(FFT_test_file));

	event_send(fopcmpl, AUDIO_STOP_CMPL);
	event_send(audio_feature, AUDIO_STOP_CMPL);
	
}

static void do_dsp_start(FIL *f_lastp, FIL *fp, char *fname)
{	
	uint32_t  	rb;
	uint8_t 	*buffer;
	uint8_t     length;
	wavhead_t *head = &last_audio_head;
	buffer = (uint8_t *)malloc(WAV_HEADER_SIZE); 		/* �������wavheader 44 �ֽ� */
	struct chunk_riff 	*riffp;
	struct chunk_fmt 	*fmtp;
	struct chunk_data 	*datap;
	
	length = rt_strlen(old_name_wav.name_buffer2);
	
	rt_memcpy(fname, old_name_wav.name_buffer2, length); 
	fname[length - 3] = 'c';
	fname[length - 2] = 's';
	fname[length - 1] = 'v';
	fname[length] = '\0';

	open(fp, fname, FA_OPEN_ALWAYS | FA_WRITE);
	debug(THREAD "%s�ļ������ɹ���\r\n", fname);
	open(f_lastp, old_name_wav.name_buffer2, FA_OPEN_EXISTING | FA_READ);
	debug(THREAD "%s�ѱ������Ƶ�ļ��򿪳ɹ���\r\n", old_name_wav.name_buffer2);
	//////////////////////////��ȡ��Ƶ���ļ�ͷ///////////////////////////////
	file_read(f_lastp, buffer, WAV_HEADER_SIZE, &rb);
	if (rb != WAV_HEADER_SIZE) {
		err("��ȡ%swavͷ����\r\n");
	}
	debug("br:%d\n", rb);
	riffp = (struct chunk_riff *)buffer;
	if(riffp->fmt != WAVE_STR) {					/* "WAVE" �ǲ���wav�ļ� */
		err("�������ļ�����wav�ļ�.\r\n");
	}
	fmtp = (struct chunk_fmt *)(buffer + 12);			/* fmt�����ʼλ�ã� ƫ����Ϊ12 */
	datap = (struct chunk_data *)(buffer + 36);		/* data �����ʼλ�ã� ƫ����Ϊ36 */
	if (datap->id != DATA_STR) {					/* "data" */ 
		err("����WAVͷ�ļ�ʧ��!\r\n");
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
	
//	debug("head->riff.id:%X %d\n", head->riff.id, head->riff.id);
//	debug("head->riff.size:%X %d\n", head->riff.size, head->riff.size);
//	debug("head->riff.fmt:%X %d\n", head->riff.fmt, head->riff.fmt);
//	
//	debug("\nhead->fmt.id:%X %d\n", head->fmt.id, head->fmt.id);
//	debug("head->fmt.size:%X %d\n", head->fmt.size, head->fmt.size);
//	debug("head->fmt.fmt:%X %d\n", head->fmt.fmt, head->fmt.fmt);
//	debug("head->fmt.channels:%X %d\n", head->fmt.channels, head->fmt.channels);
//	debug("head->fmt.sample_rate:%X %d\n", head->fmt.sample_rate, head->fmt.sample_rate);
//	debug("head->fmt.block_align:%X %d\n",head->fmt.block_align, head->fmt.block_align);
//	debug("head->fmt.bits:%X %d\n", head->fmt.bits, head->fmt.bits);
//	
//	debug("\nhead->data.id:%x %d\n", head->data.id, head->data.id);
	debug("head->data.size:%x %d\n", head->data.size, head->data.size);	
	
	old_name_wav.second = datap->size;
	
//	free(head);
		
	event_send(audio_feature, DSP_START_CMPL);
	debug(THREAD "����DSP�ļ���������¼�~\r\n");
}

static void do_dsp_read_event(FIL *fp, uint32_t *rb)
{
//	debug(THREAD "��ȡһ֡����\n");
	file_read(fp, dspdatabuf.buffer, DSP_BUFFER_SIZE*2, rb);
//	debug(THREAD "��ȡ���ݳɹ�\n");
	event_send(audio_feature, DSP_READ_EVENT_CMPL);
}

static void do_dsp_write_event(FIL *fp, uint32_t *wb)
{
//	debug(THREAD "д��һ֡����������\n");
	write(fp, dspdatabuf.feature_buffer, 
		 rt_strlen(dspdatabuf.feature_buffer), wb);
//	debug(THREAD "д�����������ɹ�\n");
	event_send(audio_feature, DSP_WRITE_EVENT_CMPL);
}

static void do_dsp_stop(FIL *fp, FIL *f_lastp, char *fname)
{
	
	close(fp);
	close(f_lastp);
	debug("fname:%s\n", fname);
	debug("old_name:%s\n", old_name_wav.name_buffer2);

	if(f_unlink(old_name_wav.name_buffer2) != 0){
		debug(THREAD "ɾ���ļ�����%d\n", f_unlink(old_name_wav.name_buffer2));
	}  else {
		debug(THREAD "%sɾ���ļ��ɹ���\n", old_name_wav.name_buffer2);
	}
	
	event_send(audio_feature, DSP_STOP_CMPL);
	debug(THREAD "���͹ر�DSP�ļ���������¼�~\r\n");
	
}


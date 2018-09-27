#include "file.h"
#include "config.h"

/**
  ******************************************************************************
  * @brief   ������д�뻺���У� ����������ˣ� �ú��������flush����ˢ����
  * @param   file: �ļ��ṹ
			 pmsg: д�뻺���е��ַ���
  * @retval  -1: ���� 
			 ��ֵ: д�뻺����ֽ��� 
  ******************************************************************************
  */
int32_t file_write(file_t file, char *pmsg)
{
	uint32_t msglength = rt_strlen(pmsg);						/* ��д���ļ��ĳ��� */
	uint32_t leftroom = BUFFERSIZE - file->bufferused;		/* �ļ������л�ʣ�Ŀռ� */
	
	assert(file);
	assert(pmsg);
	if (msglength == 0) 
		return 0;

	if(msglength >= leftroom)		/* ��д���ļ����ȴ����ļ�����Ŀ��õĳ��� */
		file->flush(file);			/* ˢһ�»��� */
	
	rt_memcpy((char*)(&file->buffer) + file->bufferused, pmsg, msglength);
	file->bufferused += msglength;
	
	return msglength;
}

/**
  ******************************************************************************
  * @brief   ˢ���棬 �������е�����д��SD����
  * @param   file: �ļ��ṹ
  * @retval  -1: ���� 
			 ��ֵ: ʵ��д���ļ����ֽ��� 
  ******************************************************************************
  */
int32_t file_flush(file_t file)
{
	FRESULT res;
	uint32_t wb;
	
	assert(file);
	if(file->oflag == 0) {
		err("file is not opened!\r\n");
		return -1;
	}
	
	res = f_write(&file->fp, &file->buffer, file->bufferused, &wb);
	if(res != FR_OK) {
		err("f_write error!\r\n");
		return -1;
	}
	debug("д��%s��%d�ֽ�\n", file->name, wb);
	file->bufferused = 0;
	if(res != FR_OK) {
		err("f_write error!\r\n");
		return -1;
	}
	
	return wb;
}

/**
  ******************************************************************************
  * @brief   �򿪸�Ŀ¼���ļ��� �ļ�����file->nameָ��
  * @param   file: �ļ��ṹ
  * @retval  -1: ���� 
			  0: �ɹ� 
  ******************************************************************************
  */
int32_t file_open(file_t file)
{
	FRESULT 	res;
	FATFS 		fs;
	DIR 		dir;

	res = f_mount(FS_SD, &fs);
	if(res != FR_OK) {
		err("f_mount error!\r\n");
		return -1;
	}
	
	res = f_opendir(&dir, "/");
	if(res != FR_OK) {
		err("f_opendir error!\r\n");
		return -1;
	}
	/* ���ļ�������ļ������ڣ��򴴽�һ�����ļ����˷�ʽ����ʹ��f_lseek�����Դ򿪵��ļ�׷������ */
	res = f_open(&file->fp, file->name, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
	if(res != FR_OK) {
		err("f_open error!\r\n");
		return -1;
	}
	
	file->oflag = 1;
	return 0;
}	

/**
  ******************************************************************************
  * @brief   �ر��ļ��� �ļ�����file->nameָ��
  * @param   file: �ļ��ṹ
  * @retval  -1: ���� 
			  0: �ɹ� 
  ******************************************************************************
  */
int32_t file_close(file_t file)
{	
	assert(file);
	if (file->flush(file) == -1) {
		err("file flush error!\r\n");
		return -1;
	}
	
	if (f_close(&file->fp) != FR_OK) {
		err("f_close error!\r\n");
		return -1;
	}
	
	file->oflag = 0;		/* �ļ��򿪱�־ */
	if(f_mount(FS_SD, RT_NULL) != FR_OK) {
		err("f_mount error!\r\n");
		return -1;
	}
	
	return 0;	
}

/**
  ******************************************************************************
  * @brief   һ���Զ�ȡBUFFER_SIZE���ֽ��ļ����ݵ��ļ�buffer��
			 ��֤����ͬ���ļ������̰߳�ȫ��
  * @param   file: �ļ��ṹ
			 pos: ��ȡ�ļ���λ��
  * @retval  -1: ���� 
			 br: ʵ�ʶ�ȡ�������� 
  ******************************************************************************
  */
int32_t file_read(file_t file, uint32_t pos)
{
	
	uint32_t br;		/* ����f_read ʵ�ʶ������ֽ��� */
	FRESULT res;
				
	assert(file);
	/* ��֮ǰˢһ�»��� */
	if (file->flush(file) == -1) {
		err("In file_read(), flush->error !\r\n");
		return -1;
	}
	
	/* �ļ���ָ�붨λ */
	if (f_lseek(&file->fp, pos) != FR_OK) {
		err("In file_read(), f_lseek error!\r\n");
		return -1;
	}
	
	/* һ���Զ��������� */
	res = f_read(&file->fp, file->buffer, BUFFERSIZE, &br);
	if( res != FR_OK) {
		err("In file_read() f_read error! errno: %d\r\n", res);
		return -1;
	}
	return br;
}


/* 
	�ļ�ע�ᣬ ע����֮�����ʹ��open/close/read/write�Ⱥ���
	file: �ļ��ṹ��
	name: �ļ���
*/
int32_t file_register(file_t file, char *name)
{
	assert(file);
	assert(name);
	file->oflag 		= 0;
	file->bufferused 	= 0;
	file->name 			= name;
	file->open 			= file_open;
	file->close 		= file_close;
	file->flush 		= file_flush;
	file->write 		= file_write;
	file->read 			= file_read;
	
	return 0;
}

/* ���ԭ��SD��������������־ͷ���0�� û�з���1 */
uint8_t check_file_name(uint8_t number)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];
	char str_number[3];
	uint8_t file_number;
	char name[5];
	
	/* �����ļ�ϵͳ */
	result = f_mount(FS_SD, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
		err("�����ļ�ϵͳʧ�� (%d)\r\n", result);
	
	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK) {
		err("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return 0;
	}
	
	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
			break;

		if (FileInf.fname[0] == '.')
			continue;

		/* �ж����ļ�������Ŀ¼ */
		if (!(FileInf.fattrib & AM_DIR))
		{
			rt_strncpy(name, FileInf.fname, 5);
			if (rt_strncmp(name, "data_", 5) == 0) {
				debug("name: %s\n", name);
				str_number[0] = FileInf.fname[5];
				str_number[1] = FileInf.fname[6];
				str_number[2] = FileInf.fname[7];
				if (str_number[0] > '9' || str_number[0] < '0' || 
					str_number[1] > '9' || str_number[1] < '0' ||
					str_number[2] > '9' || str_number[2] < '0')
					continue;
				
				debug("%c", str_number[0]);
				debug("%c", str_number[1]);
				debug("%c", str_number[2]);
				debug("\n");
				file_number = (str_number[0] - '0') * 100 + (str_number[1] - '0')* 10 + str_number[2] - '0';
				if (file_number == number) {
					f_mount(FS_SD, RT_NULL);
					debug("number: %d", file_number);
					return 0;
				}	
			}
			else
				continue;

		}
	}

	/* ж���ļ�ϵͳ */
	f_mount(FS_SD, RT_NULL);
	return 1;
}

/* ��ʾ��Ŀ¼�µ��ļ��б� */
void root_dir(void)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];
	
	/* �����ļ�ϵͳ */
	result = f_mount(FS_SD, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
		err("�����ļ�ϵͳʧ�� (%d)\r\n", result);

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK) {
		err("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return;
	}
	
	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	
	debug("����        |  �ļ���С | ���ļ��� | ���ļ���\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
			break;

		if (FileInf.fname[0] == '.')
			continue;

		/* �ж����ļ�������Ŀ¼ */
		if (FileInf.fattrib & AM_DIR)
			debug("(0x%02d)Ŀ¼  ", FileInf.fattrib);
		else
			debug("(0x%02d)�ļ�  ", FileInf.fattrib);

		/* ��ӡ�ļ���С, ���4G */
		debug(" %10d", FileInf.fsize);
		debug("  %s |", FileInf.fname);					/* ���ļ��� */
		debug("  %s\r\n", (char *)FileInf.lfname);		/* ���ļ��� */
	}

	/* ж���ļ�ϵͳ */
	f_mount(FS_SD, RT_NULL);
}

int32_t get_file(char buf[64], int size, int fileno)
{
	/* ������ʹ�õľֲ�����ռ�ý϶࣬���޸������ļ�����֤��ջ�ռ乻�� */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	uint16_t n = 0;
	char lfname[256];
	
	/* �����ļ�ϵͳ */
	result = f_mount(FS_SD, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
		debug("�����ļ�ϵͳʧ�� (%d)\r\n", result);

	/* �򿪸��ļ��� */
	result = f_opendir(&DirInf, "/"); /* ���������������ӵ�ǰĿ¼��ʼ */
	if (result != FR_OK) {
		debug("�򿪸�Ŀ¼ʧ�� (%d)\r\n", result);
		return -1;
	}

	/* ��ȡ��ǰ�ļ����µ��ļ���Ŀ¼ */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	for (cnt = 0; ;cnt++) {
		debug("cnt: %d\n", cnt);
		result = f_readdir(&DirInf, &FileInf); 		/* ��ȡĿ¼��������Զ����� */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}
		if (FileInf.fname[0] == '.')
			continue;

		/* �ж����ļ�������Ŀ¼ */
		if (!(FileInf.fattrib & AM_DIR)) {
			if (n == fileno) {
				debug("  %s \n", FileInf.fname);				/* ���ļ��� */
				rt_strncpy(buf, FileInf.fname, sizeof(FileInf.fname));
				f_mount(FS_SD, RT_NULL);
				return sizeof(FileInf.fname);
			}
			else if (n < fileno)
				n++;
			else {
				f_mount(FS_SD, RT_NULL);
				return 0;		/* û���ļ��� */
			}
		}
	}

	/* ж���ļ�ϵͳ */
	f_mount(FS_SD, RT_NULL);
	return 0;
}


	


#include "file.h"
#include "config.h"

/**
  ******************************************************************************
  * @brief   将数据写入缓存中， 如果缓存满了， 该函数会调用flush来来刷缓存
  * @param   file: 文件结构
			 pmsg: 写入缓存中的字符串
  * @retval  -1: 出错 
			 正值: 写入缓存的字节数 
  ******************************************************************************
  */
int32_t file_write(file_t file, char *pmsg)
{
	uint32_t msglength = rt_strlen(pmsg);						/* 待写入文件的长度 */
	uint32_t leftroom = BUFFERSIZE - file->bufferused;		/* 文件缓存中还剩的空间 */
	
	assert(file);
	assert(pmsg);
	if (msglength == 0) 
		return 0;

	if(msglength >= leftroom)		/* 代写的文件长度大于文件缓存的可用的长度 */
		file->flush(file);			/* 刷一下缓存 */
	
	rt_memcpy((char*)(&file->buffer) + file->bufferused, pmsg, msglength);
	file->bufferused += msglength;
	
	return msglength;
}

/**
  ******************************************************************************
  * @brief   刷缓存， 将缓存中的数据写入SD卡中
  * @param   file: 文件结构
  * @retval  -1: 出错 
			 正值: 实际写入文件的字节数 
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
	debug("写入%s中%d字节\n", file->name, wb);
	file->bufferused = 0;
	if(res != FR_OK) {
		err("f_write error!\r\n");
		return -1;
	}
	
	return wb;
}

/**
  ******************************************************************************
  * @brief   打开根目录下文件， 文件名由file->name指定
  * @param   file: 文件结构
  * @retval  -1: 出错 
			  0: 成功 
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
	/* 打开文件，如果文件不存在，则创建一个新文件，此方式可以使用f_lseek函数对打开的文件追加数据 */
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
  * @brief   关闭文件， 文件名由file->name指定
  * @param   file: 文件结构
  * @retval  -1: 出错 
			  0: 成功 
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
	
	file->oflag = 0;		/* 文件打开标志 */
	if(f_mount(FS_SD, RT_NULL) != FR_OK) {
		err("f_mount error!\r\n");
		return -1;
	}
	
	return 0;	
}

/**
  ******************************************************************************
  * @brief   一次性读取BUFFER_SIZE个字节文件数据到文件buffer中
			 保证对相同的文件操作线程安全。
  * @param   file: 文件结构
			 pos: 读取文件的位置
  * @retval  -1: 出错 
			 br: 实际读取到的数据 
  ******************************************************************************
  */
int32_t file_read(file_t file, uint32_t pos)
{
	
	uint32_t br;		/* 调用f_read 实际读到的字节数 */
	FRESULT res;
				
	assert(file);
	/* 读之前刷一下缓存 */
	if (file->flush(file) == -1) {
		err("In file_read(), flush->error !\r\n");
		return -1;
	}
	
	/* 文件读指针定位 */
	if (f_lseek(&file->fp, pos) != FR_OK) {
		err("In file_read(), f_lseek error!\r\n");
		return -1;
	}
	
	/* 一次性读满缓冲区 */
	res = f_read(&file->fp, file->buffer, BUFFERSIZE, &br);
	if( res != FR_OK) {
		err("In file_read() f_read error! errno: %d\r\n", res);
		return -1;
	}
	return br;
}


/* 
	文件注册， 注册完之后才能使用open/close/read/write等函数
	file: 文件结构体
	name: 文件名
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

/* 如果原来SD卡里面有这个名字就返回0， 没有返回1 */
uint8_t check_file_name(uint8_t number)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];
	char str_number[3];
	uint8_t file_number;
	char name[5];
	
	/* 挂载文件系统 */
	result = f_mount(FS_SD, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
		err("挂载文件系统失败 (%d)\r\n", result);
	
	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK) {
		err("打开根目录失败 (%d)\r\n", result);
		return 0;
	}
	
	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
			break;

		if (FileInf.fname[0] == '.')
			continue;

		/* 判断是文件还是子目录 */
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

	/* 卸载文件系统 */
	f_mount(FS_SD, RT_NULL);
	return 1;
}

/* 显示根目录下的文件列表 */
void root_dir(void)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	char lfname[256];
	
	/* 挂载文件系统 */
	result = f_mount(FS_SD, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
		err("挂载文件系统失败 (%d)\r\n", result);

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK) {
		err("打开根目录失败 (%d)\r\n", result);
		return;
	}
	
	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	
	debug("属性        |  文件大小 | 短文件名 | 长文件名\r\n");
	for (cnt = 0; ;cnt++)
	{
		result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
			break;

		if (FileInf.fname[0] == '.')
			continue;

		/* 判断是文件还是子目录 */
		if (FileInf.fattrib & AM_DIR)
			debug("(0x%02d)目录  ", FileInf.fattrib);
		else
			debug("(0x%02d)文件  ", FileInf.fattrib);

		/* 打印文件大小, 最大4G */
		debug(" %10d", FileInf.fsize);
		debug("  %s |", FileInf.fname);					/* 短文件名 */
		debug("  %s\r\n", (char *)FileInf.lfname);		/* 长文件名 */
	}

	/* 卸载文件系统 */
	f_mount(FS_SD, RT_NULL);
}

int32_t get_file(char buf[64], int size, int fileno)
{
	/* 本函数使用的局部变量占用较多，请修改启动文件，保证堆栈空间够用 */
	FRESULT result;
	FATFS fs;
	DIR DirInf;
	FILINFO FileInf;
	uint32_t cnt = 0;
	uint16_t n = 0;
	char lfname[256];
	
	/* 挂载文件系统 */
	result = f_mount(FS_SD, &fs);	/* Mount a logical drive */
	if (result != FR_OK)
		debug("挂载文件系统失败 (%d)\r\n", result);

	/* 打开根文件夹 */
	result = f_opendir(&DirInf, "/"); /* 如果不带参数，则从当前目录开始 */
	if (result != FR_OK) {
		debug("打开根目录失败 (%d)\r\n", result);
		return -1;
	}

	/* 读取当前文件夹下的文件和目录 */
	FileInf.lfname = lfname;
	FileInf.lfsize = 256;
	for (cnt = 0; ;cnt++) {
		debug("cnt: %d\n", cnt);
		result = f_readdir(&DirInf, &FileInf); 		/* 读取目录项，索引会自动下移 */
		if (result != FR_OK || FileInf.fname[0] == 0)
		{
			break;
		}
		if (FileInf.fname[0] == '.')
			continue;

		/* 判断是文件还是子目录 */
		if (!(FileInf.fattrib & AM_DIR)) {
			if (n == fileno) {
				debug("  %s \n", FileInf.fname);				/* 短文件名 */
				rt_strncpy(buf, FileInf.fname, sizeof(FileInf.fname));
				f_mount(FS_SD, RT_NULL);
				return sizeof(FileInf.fname);
			}
			else if (n < fileno)
				n++;
			else {
				f_mount(FS_SD, RT_NULL);
				return 0;		/* 没有文件了 */
			}
		}
	}

	/* 卸载文件系统 */
	f_mount(FS_SD, RT_NULL);
	return 0;
}


	


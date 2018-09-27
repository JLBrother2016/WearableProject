#ifndef _FILE_H_
#define _FILE_H_

#include "ff.h"
#include <stm32f4xx.h>
//#include <rtthread.h>

#define BUFFERSIZE 4000

#ifdef __cplusplus
extern "C" {
#endif

typedef struct devfile* file_t;
struct devfile
{
	int32_t (*open)(file_t file);
	int32_t (*close)(file_t file);
	int32_t (*read)(file_t file, uint32_t pos);
	int32_t (*write)(file_t file, char *pmsg);
	int32_t (*flush)(file_t file);

	FIL 					fp;						/* 文件指针 */
	char 					*name;					//文件名
	uint32_t 				bufferused;				//已经使用的缓存
	char					buffer[BUFFERSIZE];		//文件缓存
	uint8_t 				oflag;					//文件是否打开的标志
};

extern int32_t file_register(file_t file, char *name);
void root_dir(void);
uint8_t check_file_name(uint8_t number);
int32_t get_file(char buf[64], int size, int fileno);

#ifdef __cplusplus
}
#endif

#endif

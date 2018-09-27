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

	FIL 					fp;						/* �ļ�ָ�� */
	char 					*name;					//�ļ���
	uint32_t 				bufferused;				//�Ѿ�ʹ�õĻ���
	char					buffer[BUFFERSIZE];		//�ļ�����
	uint8_t 				oflag;					//�ļ��Ƿ�򿪵ı�־
};

extern int32_t file_register(file_t file, char *name);
void root_dir(void);
uint8_t check_file_name(uint8_t number);
int32_t get_file(char buf[64], int size, int fileno);

#ifdef __cplusplus
}
#endif

#endif

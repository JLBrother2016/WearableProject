#ifndef _EASY_RTTHREAD_H_
#define _EASY_RTTHREAD_H_

//#include "config.h"
#include <rtthread.h>
#include <stdint.h>
#include "ff.h"

#define 	scheduler_lock 			rt_enter_critical		//锁调度器
#define 	scheduler_unlock		rt_exit_critical		//解锁调度器


struct thread_opt
{
    void       		*parameter;
    rt_uint32_t 	stack_size;
    rt_uint32_t 	tick;
};

/* 用互斥量来保护标志量 */
typedef struct mutex_flag {
	uint8_t 	flag;
	rt_mutex_t mutex;
} mutex_flag_t;

//记录数据标志
typedef struct recording_flags {
	uint8_t sensors 	: 1;
	uint8_t mpu 		: 1;
	uint8_t audio		: 1;
	uint8_t oled		: 1;
	rt_mutex_t			mutex;
} recflg_t;

int32_t thread_create(const char *name, void (*entry)(void *parameter), rt_uint8_t priority, struct thread_opt *opt, rt_thread_t *tid);
rt_mutex_t mutex_create(const char *name);
int32_t mutex_lock(rt_mutex_t mutex);
int32_t mutex_unlock(rt_mutex_t mutex);
int32_t mutex_init(rt_mutex_t mutex, const char *name);
int32_t mount(void);
rt_event_t event_create(const char *name);
int32_t event_delete(rt_event_t event);
int32_t event_send(rt_event_t event, rt_uint32_t set);
int32_t event_recv(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_uint32_t *recv);
rt_err_t event_recv_timeout(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_int32_t timeout, rt_uint32_t *recv);

FRESULT open(FIL *fp, const TCHAR *path, BYTE mode);
FRESULT close(FIL *fp);
FRESULT write(FIL *fp, const void *buff, UINT btw, UINT *bw);
FRESULT file_read(FIL *fp, void *buff, UINT btw, UINT *bw);
FRESULT sync(FIL *fp);
FRESULT rename(const TCHAR *path_old, const TCHAR *path_new);
FRESULT lseek(FIL *fp, DWORD ofs);	

#endif //_EASY_RTTHREAD_H_

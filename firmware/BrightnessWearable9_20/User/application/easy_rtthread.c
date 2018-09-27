#include "easy_rtthread.h"
#include "fatfserr.h"
#include "config.h"

/* 
 * ��̬�����߳� 
 */
int32_t thread_create(const char *name, void (*entry)(void *parameter), rt_uint8_t priority, struct thread_opt *opt, rt_thread_t* tid)
{
//	rt_thread_t _tid;
	
	if (opt == RT_NULL)
		*tid = rt_thread_create(name, entry, RT_NULL, 4096, priority, 10); 			/* Ĭ������ */
	else
		*tid = rt_thread_create(name, entry, opt->parameter, opt->stack_size, priority, opt->tick);
	
	if (*tid == RT_NULL) {
		rt_kprintf("creating %s thread failed!\r\n", name);
		return -1;
	}
	
    rt_thread_startup(*tid);
    
//    *tid = _tid;
	return 0;
}

/*
 *=========	��������صĺ���=================
 * ��̬������������ IPC��������FIFO��ʽ
*/
rt_mutex_t mutex_create(const char *name)
{
	rt_mutex_t ret;
	
	ret = rt_mutex_create(name, RT_IPC_FLAG_FIFO);
	if (ret == RT_NULL)
		err("In mutex_create(), ret == NULL");
	
	return ret;	
}

/* 
 * ɾ�������� 
 */
int32_t mutex_delete(rt_mutex_t mutex)
{
	if (rt_mutex_delete(mutex) != RT_EOK) {
		err("In mutex_delete(), error!\r\n");
		return -1;
	}
	else
		return 0;
}

/* 
 * ���������� 
 */
int32_t mutex_lock(rt_mutex_t mutex)
{	
	if (rt_mutex_take(mutex, RT_WAITING_FOREVER) != RT_EOK) {
		err("In mutex_lock(), error!\r\n");
		return -1;
	}
	else
		return 0;
}

/* 
 * ���������� 
 */
int32_t mutex_unlock(rt_mutex_t mutex)
{
	if (rt_mutex_release(mutex) != RT_EOK) {
		err("In mutex_unlock(), error!\r\n");
		return -1;
	}
	else
		return 0;
}

/* 
 * ��̬��ʼ������
 * IPC��������FIFO��ʽ 
 */
int32_t mutex_init(rt_mutex_t mutex, const char *name)
{	
	if(rt_mutex_init(mutex, name, RT_IPC_FLAG_FIFO) != RT_EOK)
	{
		err("In mutex_init(), error!\r\n");
		return -1;
	}
	else
		return 0;
}

/*
 *��============�¼����==================
 *����̬�����¼��� FIFO��ʽ
��*/
rt_event_t event_create(const char *name)
{
	rt_event_t ret;
	
	ret = rt_event_create(name, RT_IPC_FLAG_FIFO);
	if (ret == RT_NULL)
		err("In event_create(), error!\r\n");
	
	return ret;
}

/* 
 * ɾ���¼� 
 */
int32_t event_delete(rt_event_t event)
{
	if (rt_event_delete(event) != RT_EOK) {
		err("In event_delete(), error!\r\n");
		return -1;
	}
	
	return 0;
}

/* 
 * �����¼� 
 */
int32_t event_send(rt_event_t event, rt_uint32_t set)
{
	if (rt_event_send(event, set) != RT_EOK) {
		err("In event_send(), error!\r\n");
		return -1;
	}
	
	return 0;
}

/*
 * ���������¼�
 */
int32_t event_recv(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_uint32_t *recv)
{
	if (rt_event_recv(event, set, option, RT_WAITING_FOREVER, recv) != RT_EOK) {
		err("event_recv failed!\r\n");
		return -1;
	}
	
	return 0;
}

//��ʱ�����¼�
rt_err_t event_recv_timeout(rt_event_t event, rt_uint32_t set, rt_uint8_t option, rt_int32_t timeout, rt_uint32_t *recv)
{
	rt_err_t res;
	
	res = rt_event_recv(event, set, option, timeout, recv);
	if (res == RT_ERROR) {
		err("event_recv_timeout failed!\r\n");	
	} 
	
	return res;
}


/*
 * �����ļ�ϵͳ
 */
extern FATFS fs;
int32_t mount(void)
{
	FRESULT 	res;
	
	if ((res = f_mount(FS_SD, &fs)) != FR_OK) {
		rt_kprintf("����FATFS�ļ�ϵͳ����, %s(res: %d)\n", fserr(res), res);
		return -1;
	} else {
		rt_kprintf("����FATFS�ļ�ϵͳ�ɹ�\n");
		return 0;
	}
}

//�����ļ���������������������ʽ�̱߳��л�
FRESULT open(FIL *fp, const TCHAR *path, BYTE mode)
{
	FRESULT  res;
	
	scheduler_lock();
	res = f_open(fp, path, mode);
	if (res != RT_EOK) {
		err("��/�����ļ�%sʧ��, %s(res: %d)\n", path, fserr(res), res);
		while (1);
	}
	scheduler_unlock();
	
	return res;	
}

FRESULT close(FIL *fp)
{
	FRESULT  res;
	
	scheduler_lock();
	res = f_close(fp);
	if (res != RT_EOK) {
		err("�ر��ļ�ʧ��!\r\n");
		while (1);
	}
	scheduler_unlock();
	
	return res;
}


FRESULT write(FIL *fp, const void *buff, UINT btw, UINT *bw)
{
	FRESULT  res;
	
	scheduler_lock();
	res = f_write(fp, buff, btw, bw);
	if (res != RT_EOK || *bw != btw) {
		err("�ļ�д����̴���, %s(res: %d), bw: %d, btw: %d\r\n", fserr(res), res, *bw, btw);
		while (1);
	}
	scheduler_unlock();
	
	return res;
}

FRESULT file_read(FIL *fp, void *buff, UINT btw, UINT *bw)
{
	FRESULT  res;
    
	scheduler_lock();
	res = f_read(fp, buff, btw, bw);
    if (res != RT_EOK)
    {
        err("���ļ�ʧ��!, %s(res: %d)",fserr(res), res);
        while (1);
    }
	scheduler_unlock();
	
	return res;
}

FRESULT lseek(FIL *fp, DWORD ofs)
{
	FRESULT  res;
	
	scheduler_lock();
	res = f_lseek(fp, ofs);
	if (res != RT_EOK) {
		err("lseek error, %s(res: %d)\r\n", fserr(res), res);
		while (1);
	}
	scheduler_unlock();
	
	return res;
}


FRESULT sync(FIL *fp)
{
	FRESULT  res;
	
	scheduler_lock();
	res = f_sync(fp);
	if (res != RT_EOK) {
		err("�ļ�д����̴���, %s(res: %d)\n", fserr(res), res);
		while (1);
	}
	scheduler_unlock();
	
	return res;
}

FRESULT rename(const TCHAR *path_old, const TCHAR *path_new)
{
	FRESULT  res;
	
	scheduler_lock();
	res = f_rename(path_old, path_new);
	if (res != RT_EOK) {
		err("�������ļ�����, %s(res: %d)\n", fserr(res), res);
		while (1);
	}
	scheduler_unlock();
	
	return res;
}
















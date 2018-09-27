/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2013        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control module to the FatFs module with a defined API.        */
/*-----------------------------------------------------------------------*/

#include <rtthread.h>
#include "diskio.h"			/* FatFs lower layer API */
#include "sdio_sd.h"		/* SD���ײ����� */
#include "config.h"
#include "rtc.h"

#define SECTOR_SIZE		512

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS SD_disk_initialize(void)
{
	SD_CardInfo SDCardInfo;
	SD_Error Status;

	Status = SD_Init();

	if (Status == SD_OK) {
		return RES_OK;
	} else {
		err("SD_Init failed! Status: %d\r\n", Status);
		return STA_NODISK;
	}
}

/*
	��ʼ������������
	pdrv: �ƶ�Ҫ��ʼ�����߼��������ţ� ���̷��� ȡֵFS_SD
	Ӧ�ó���Ӧ�õ��ô˺�����������ϵ�FAT�ṹ���ܻ��𻵣�
	�����Ҫ���³�ʼ���ļ�ϵͳ������ʹ��f_mount������
	��FATFSģ���Ͼ�ע�ᴦ��ʱ���øú������Կ����豸�ĸı䣻��
	�˺�����FATFS���ھ�ʱ���ã�Ӧ�ó���Ӧ����FATFS�ʱʹ�ô˺���
*/
DSTATUS disk_initialize(BYTE pdrv)
{
	DSTATUS stat;

	switch (pdrv) 
	{
		case FS_SD :
			stat = SD_disk_initialize();	/* ��ʼ��SD */
			return stat;

		case FS_NAND :
			stat = STA_NOINIT;
			return stat;
	}
	
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Get Disk Status                                                       */
/*-----------------------------------------------------------------------*/
/*
	���ص�ǰ������������״̬
	pdrv Ҫȷ�ϵ��߼���������
*/
DSTATUS disk_status(BYTE pdrv)
{
	DSTATUS stat;

	switch (pdrv)
	{
	case FS_SD :
		stat = 0;	/* ��װ����������Ͳ�֧��д������ʱ������0 */
		return stat;

	case FS_NAND :
		return stat;
	}
	return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/
/*
	�Ӵ����������϶�����
pdrv: ָ���߼���������
buff: ָ��洢��ȡ�����ֽ������ָ�룬��ҪΪ����ȡ�ֽ����Ĵ�С������ͳ�Ƶ�������С����Ҫ��
sector: ָ����ʼ�������߼��飨LBA���ϵĵ�ַ
count: ��Ҫ��ȡ��������ȡֵ1-128
*/
DRESULT disk_read(BYTE pdrv, BYTE *buff, DWORD sector, BYTE count)
{
	DRESULT res;

	switch (pdrv) {
		case FS_SD :
		{
			SD_Error Status = SD_OK;
			if (count == 1)
				Status = SD_ReadBlock(buff, sector << 9 , SECTOR_SIZE);
			else
				Status = SD_ReadMultiBlocks(buff, sector << 9 , SECTOR_SIZE, count);
			if (Status != SD_OK)
				return RES_ERROR;

		#ifdef SD_DMA_MODE
			/* SDIO������DMAģʽ����Ҫ������DMA�����Ƿ���� */
			Status = SD_WaitReadOperation();
			if (Status != SD_OK)
				return RES_ERROR;

			while(SD_GetStatus() != SD_TRANSFER_OK);
		#endif

			return RES_OK;	/* �����ɹ� */
		}
	}
	return RES_PARERR;		/* �Ƿ����� */
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/*
	�����д��һ����������
pdrv: ָ�����߼���������
buffer: ָ��Ҫд���ֽ�����ָ��
sector: ָ����ʼ�������߼��飨LBA���ϵ�ַ
count: ָ��Ҫд�����������ȡֵ1-128
*/
#if _USE_WRITE
DRESULT disk_write(BYTE pdrv, const BYTE *buff,	DWORD sector, BYTE count)
{
	DRESULT res;

	switch (pdrv) 
	{
		case FS_SD :
		{
			SD_Error Status = SD_OK;

			if (count == 1)
			{
				Status = SD_WriteBlock((uint8_t *)buff, sector << 9 ,SECTOR_SIZE);

				if (Status != SD_OK)
				{
					return RES_ERROR;
				}

			#ifdef SD_DMA_MODE
				/* SDIO������DMAģʽ����Ҫ������DMA�����Ƿ���� */
				Status = SD_WaitReadOperation();
				if (Status != SD_OK)
				{
					return RES_ERROR;
				}
				while(SD_GetStatus() != SD_TRANSFER_OK);
			#endif
				return RES_OK;
			}
			else
			{
				/* �˴��������ʣ� �����������д count �����������1��block�޷�д�� */
				Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count);
				//Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);

				if (Status != SD_OK)
				{
					return RES_ERROR;
				}

			#ifdef SD_DMA_MODE
				/* SDIO������DMAģʽ����Ҫ������DMA�����Ƿ���� */
				Status = SD_WaitReadOperation();
				if (Status != SD_OK)
				{
					return RES_ERROR;
				}
				while(SD_GetStatus() != SD_TRANSFER_OK);
			#endif

				return RES_OK;
			}
		}
	}
	
	return RES_PARERR;		/* �Ƿ����� */
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
/* 
	�����豸ָ�����Ժͳ��˶�/д�������� 
pdrv:ָ���߼���������
cmd:ָ���������
buffer:ָ��������������ָ�룬ȡ����������룬������ʱ��ָ��һ��NULLָ��
*/
#if _USE_IOCTL
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff)
{
	DRESULT res;

	switch (pdrv) 
	{
		case FS_SD :
			res = RES_OK;
			return res;
	}
	
	return RES_PARERR;
}
#endif

/*
*********************************************************************************************************
*	�� �� ��: get_fattime
*	����˵��: ���ϵͳʱ�䣬���ڸ�д�ļ��Ĵ������޸�ʱ�䡣
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
DWORD get_fattime(void)
{
	/* �����ȫ��ʱ�ӣ��ɰ�����ĸ�ʽ����ʱ��ת��. ���������2013-01-01 00:00:00 */
	rtc_t 				rtc;
	get_rtc_info(&rtc);
	return	  ((DWORD)(rtc.year - 1980) << 25)	/* Year = 2013 */
			| ((DWORD)rtc.month << 21)				/* Month = 1 */
			| ((DWORD)rtc.day << 16)				/* Day_m = 1*/
			| ((DWORD)rtc.hour << 11)				/* Hour = 0 */
			| ((DWORD)rtc.min << 5)				/* Min = 0 */
			| ((DWORD)rtc.sec >> 1);				/* Sec = 0 */
}


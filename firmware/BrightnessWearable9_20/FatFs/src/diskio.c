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
#include "sdio_sd.h"		/* SD卡底层驱动 */
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
	初始化磁盘驱动器
	pdrv: 制定要初始化的逻辑驱动器号， 即盘符， 取值FS_SD
	应用程序不应该调用此函数，否则卷上的FAT结构可能会损坏；
	如果需要重新初始化文件系统，可以使用f_mount函数；
	在FATFS模块上卷注册处理时调用该函数可以控制设备的改变；、
	此函数在FATFS挂在卷时调用，应用程序不应该在FATFS活动时使用此函数
*/
DSTATUS disk_initialize(BYTE pdrv)
{
	DSTATUS stat;

	switch (pdrv) 
	{
		case FS_SD :
			stat = SD_disk_initialize();	/* 初始化SD */
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
	返回当前磁盘驱动器的状态
	pdrv 要确认的逻辑驱动器号
*/
DSTATUS disk_status(BYTE pdrv)
{
	DSTATUS stat;

	switch (pdrv)
	{
	case FS_SD :
		stat = 0;	/* 安装磁盘驱动后和不支持写保护的时候总是0 */
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
	从磁盘驱动器上读扇区
pdrv: 指定逻辑驱动器号
buff: 指向存储读取数据字节数组的指针，需要为所读取字节数的大小，扇区统计的扇区大小是需要的
sector: 指定起始扇区的逻辑块（LBA）上的地址
count: 需要读取的扇区数取值1-128
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
			/* SDIO工作在DMA模式，需要检查操作DMA传输是否完成 */
			Status = SD_WaitReadOperation();
			if (Status != SD_OK)
				return RES_ERROR;

			while(SD_GetStatus() != SD_TRANSFER_OK);
		#endif

			return RES_OK;	/* 操作成功 */
		}
	}
	return RES_PARERR;		/* 非法参数 */
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/
/*
	向磁盘写入一个或多个扇区
pdrv: 指定的逻辑驱动器号
buffer: 指定要写入字节数组指针
sector: 指定起始扇区的逻辑块（LBA）上地址
count: 指定要写入的扇区数，取值1-128
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
				/* SDIO工作在DMA模式，需要检查操作DMA传输是否完成 */
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
				/* 此处存在疑问： 扇区个数如果写 count ，将导致最后1个block无法写入 */
				Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count);
				//Status = SD_WriteMultiBlocks((uint8_t *)buff, sector << 9 ,SECTOR_SIZE, count + 1);

				if (Status != SD_OK)
				{
					return RES_ERROR;
				}

			#ifdef SD_DMA_MODE
				/* SDIO工作在DMA模式，需要检查操作DMA传输是否完成 */
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
	
	return RES_PARERR;		/* 非法参数 */
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/
/* 
	控制设备指定特性和除了读/写外的杂项功能 
pdrv:指定逻辑驱动器号
cmd:指定命令代码
buffer:指定参数韩冲区的指针，取决于命令代码，不适用时，指定一个NULL指针
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
*	函 数 名: get_fattime
*	功能说明: 获得系统时间，用于改写文件的创建和修改时间。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
DWORD get_fattime(void)
{
	/* 如果有全局时钟，可按下面的格式进行时钟转换. 这个例子是2013-01-01 00:00:00 */
	rtc_t 				rtc;
	get_rtc_info(&rtc);
	return	  ((DWORD)(rtc.year - 1980) << 25)	/* Year = 2013 */
			| ((DWORD)rtc.month << 21)				/* Month = 1 */
			| ((DWORD)rtc.day << 16)				/* Day_m = 1*/
			| ((DWORD)rtc.hour << 11)				/* Hour = 0 */
			| ((DWORD)rtc.min << 5)				/* Min = 0 */
			| ((DWORD)rtc.sec >> 1);				/* Sec = 0 */
}


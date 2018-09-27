#include "usbd_msc_mem.h"
#include "usb_conf.h"
#include "sdio_sd.h"
#include "config.h"

//#define mass_printf	printf
#define mass_printf(...)

#define STORAGE_LUN_NBR                  1 

/*	自己定义的一个标记USB状态的寄存器
 *  bit0: 表示电脑正在向SD卡写入数据
 * 	bit1: 表示电脑正从SD卡读出数据
 * 	bit2: SD卡写数据错误标志位
 *  bit3: SD卡读数据错误标志位
 * 	bit4: 表示电脑有轮询操作(表明连接还保持着)
*/
volatile uint8_t usb_status_reg = 0;


/* USB Mass storage 标准查询数据(每个lun占36字节) */
const int8_t  STORAGE_Inquirydata[] = {
	/* LUN 0 */
	0x00,		
	0x80,		
	0x02,		
	0x02,
	(USBD_STD_INQUIRY_LENGTH - 5),
	0x00,
	0x00,	
	0x00,
	'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
	'm', 'i', 'c', 'r', 'o', 'S', 'D', ' ', /* Product      : 16 Bytes */
	'F', 'l', 'a', 's', 'h', ' ', ' ', ' ',
	'1', '.', '0' ,'0',                     /* Version      : 4 Bytes */
}; 



/** @defgroup STORAGE_Private_FunctionPrototypes
  * @{
  */ 
int8_t STORAGE_Init (uint8_t lun);
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size);
int8_t STORAGE_IsReady(uint8_t lun);
int8_t STORAGE_IsWriteProtected(uint8_t lun);
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
int8_t STORAGE_GetMaxLun(void);


/* USB Device 用户回调函数接口 */
USBD_STORAGE_cb_TypeDef USBD_MICRO_SDIO_fops =
{
	STORAGE_Init,
	STORAGE_GetCapacity,
	STORAGE_IsReady,
	STORAGE_IsWriteProtected,
	STORAGE_Read,
	STORAGE_Write,
	STORAGE_GetMaxLun,
	(int8_t *)STORAGE_Inquirydata,
};

USBD_STORAGE_cb_TypeDef  *USBD_STORAGE_fops = &USBD_MICRO_SDIO_fops;

__IO uint32_t count = 0;


int8_t STORAGE_Init(uint8_t lun)
{
	if(SD_Init() != 0) {
		err("SD_Init failed!\r\n");
		return -1; 
	}
		
	return 0;
}

/**
  * @brief  获取存储设备的容量和块大小
  * @param  lun : logical unit number
  * @param  block_num :  number of physical block
  * @param  block_size : size of a physical block
  * @retval Status
  */
int8_t STORAGE_GetCapacity(uint8_t lun, uint32_t *block_num, uint32_t *block_size)
{ 
	SD_CardInfo SDCardInfo;
	
	SD_GetCardInfo(&SDCardInfo); 
	if (SD_GetStatus() != 0 ) 
		return -1;
	
	*block_size =  512;  
	*block_num =  SDCardInfo.CardCapacity / 512;  
	return 0;
}

/**
  * @brief  check whether the medium is ready
  * @param  lun : logical unit number
  * @retval Status
  */
int8_t STORAGE_IsReady(uint8_t lun)
{
	static int8_t last_status = 0;

	if (last_status  < 0) {
		SD_Init();
		last_status = 0;
	}
	if (SD_GetStatus() != 0) {
		last_status = -1;
		usb_status_reg &= ~(1 << 4);
		return -1; 
	}
	if (SD_Init() != 0) {
		usb_status_reg &= ~(1 << 4);
		return -1;
	}
	
	usb_status_reg |= 1 << 4; 		/* 标记轮询 */
	return 0;
}

/**
  * @brief  check whether the medium is write-protected
  * @param  lun : logical unit number
  * @retval Status 0： 没有写保护
  */
int8_t STORAGE_IsWriteProtected(uint8_t lun)
{
	return  0;
}

/**
  * @brief  从存储设备读取数据
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to save data
  * @param  blk_addr :  address of 1st block to be read
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Read(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	usb_status_reg |= 1 << 1;
	if( SD_ReadMultiBlocks(buf, blk_addr * 512, 512, blk_len) != 0) {
		usb_status_reg |= 1 << 3;
		err("SD ReadMultiBlocks");
		return -1;
	}
	usb_status_reg |= ~(1 << 3);
	SD_WaitReadOperation();
	while (SD_GetStatus() != SD_TRANSFER_OK);
	usb_status_reg &= ~(1 << 1);
	return 0;
}

/**
  * @brief  Write data to the medium
  * @param  lun : logical unit number
  * @param  buf : Pointer to the buffer to write from
  * @param  blk_addr :  address of 1st block to be written
  * @param  blk_len : nmber of blocks to be read
  * @retval Status
  */
int8_t STORAGE_Write(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	usb_status_reg |= 1 << 0;			/* 标记正在写数据 */
	if (SD_WriteMultiBlocks(buf, blk_addr * 512, 512, blk_len) != 0) {
		usb_status_reg |= 1 << 2;			/* SD卡写错误 */
		err("SD WriteMultiBlocks failed!\r\n");
		return -1;
	}
	usb_status_reg &= ~(1 << 2);
	SD_WaitWriteOperation();
	while (SD_GetStatus() != SD_TRANSFER_OK);  
	usb_status_reg &= ~(1 << 0);		/* 清楚正在写数据标志 */
	return 0;
}


/**
  * @brief  Return number of supported logical unit
  * @param  None
  * @retval number of logical unit
  */
int8_t STORAGE_GetMaxLun (void)
{
	return (STORAGE_LUN_NBR - 1);
}



/*****************************END OF FILE***************************/


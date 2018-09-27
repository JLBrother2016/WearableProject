#include "dma.h"
#include "config.h"


/*
 * 配置DMA为TX模式， 从存储器到外设
 * DMA1_Stream4 
*/
void i2s_dma_set_tx_mode(uint8_t *dma_buffer0, uint8_t *dma_buffer1, uint16_t dma_buffer_size)
{
	NVIC_InitTypeDef   										NVIC_InitStructure;
	DMA_InitTypeDef  										DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);					/* DMA1时钟使能  */
	DMA_DeInit(DMA1_Stream4);												
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE);						/* 等待DMA1_Stream4 可配置 */
//	DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_FEIF4 | DMA_IT_DMEIF4 | DMA_IT_TEIF4 | DMA_IT_HTIF4 | DMA_IT_TCIF4);			/* 清空DMA1_Stream4上所有中断标志 */
	
	/* 配置DMA Stream */
	DMA_InitStructure.DMA_Channel 							= 	DMA_Channel_0;  				/* 通道0 SPI2_TX通道 */
	DMA_InitStructure.DMA_PeripheralBaseAddr 				= 	(uint32_t)&SPI2->DR;			/* 外设地址为:(u32)&SPI2->DR */
	DMA_InitStructure.DMA_Memory0BaseAddr 					= 	(uint32_t)dma_buffer0;			/* DMA 存储器0地址 */
	DMA_InitStructure.DMA_DIR 								= 	DMA_DIR_MemoryToPeripheral;		/* 存储器到外设模式 */
	DMA_InitStructure.DMA_BufferSize 						= 	dma_buffer_size;				/* 数据传输量(字节)  */
	DMA_InitStructure.DMA_PeripheralInc 					= 	DMA_PeripheralInc_Disable;		/* 外设非增量模式 */
	DMA_InitStructure.DMA_MemoryInc 						= 	DMA_MemoryInc_Enable;			/* 存储器增量模式 */
	DMA_InitStructure.DMA_PeripheralDataSize 				= 	DMA_PeripheralDataSize_HalfWord;/* 外设数据长度16位 */
	DMA_InitStructure.DMA_MemoryDataSize 					= 	DMA_MemoryDataSize_HalfWord;	/* 存储器数据长度16位  */
	DMA_InitStructure.DMA_Mode 								= 	DMA_Mode_Circular;				/* 使用循环模式 */
	DMA_InitStructure.DMA_Priority 							= 	DMA_Priority_High;				/* 高优先级 */
	DMA_InitStructure.DMA_FIFOMode 							= 	DMA_FIFOMode_Disable; 			/* 不使用FIFO模式 */        
	DMA_InitStructure.DMA_FIFOThreshold 					= 	DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst 						= 	DMA_MemoryBurst_Single;			/* 外设突发单次传输 */
	DMA_InitStructure.DMA_PeripheralBurst 					= 	DMA_PeripheralBurst_Single;		/* 存储器突发单次传输 */
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);													/* 初始化DMA Stream */
	
	DMA_DoubleBufferModeConfig(DMA1_Stream4, (uint32_t)dma_buffer1, DMA_Memory_0);				/* 双缓冲模式配置 */
	DMA_DoubleBufferModeCmd(DMA1_Stream4, ENABLE);												/* 双缓冲模式开启 */
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);												/* 开启传输完成中断 */
	
	NVIC_InitStructure.NVIC_IRQChannel 						= 	DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 	2;								/* 抢占优先级0 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 	0;								/* 子优先级0 */
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= 	ENABLE;							/* 使能外部中断通道 */
	NVIC_Init(&NVIC_InitStructure);		
}


/*
 * 配置DMA为RX模式， 从外设到存储器 
 * 设置 DMA1_Stream3
*/
void i2s_dma_set_rx_mode(uint8_t *dma_buffer0, uint8_t *dma_buffer1, uint16_t dma_buffer_size)
{
	NVIC_InitTypeDef   										NVIC_InitStructure;
	DMA_InitTypeDef  										DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);					/* DMA1时钟使能  */
	DMA_DeInit(DMA1_Stream3);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);						/* 等待DMA1_Stream4 可配置 */
	DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_FEIF3 | DMA_IT_DMEIF3 | DMA_IT_TEIF3 | DMA_IT_HTIF3 | DMA_IT_TCIF3);			/* 清空DMA1_Stream4上所有中断标志 */
	
	/* 配置 DMA Stream */
	DMA_InitStructure.DMA_Channel 							= 	DMA_Channel_3;  					/* 通道3 I2S2ext_RX通道 */
	DMA_InitStructure.DMA_PeripheralBaseAddr 				= 	(uint32_t)&I2S2ext->DR;			/* 外设地址为:(u32)&I2S2ext->DR>DR 实际地址为 0x4000340C*/
	DMA_InitStructure.DMA_Memory0BaseAddr 					= 	(uint32_t)dma_buffer0;				/* DMA 存储器0地址 */
	DMA_InitStructure.DMA_DIR 								= 	DMA_DIR_PeripheralToMemory;			/* 外设到存储器模式 */
	DMA_InitStructure.DMA_BufferSize 						= 	dma_buffer_size;					/* 数据传输量（字节） */
	DMA_InitStructure.DMA_PeripheralInc 					= 	DMA_PeripheralInc_Disable;			/* 外设非增量模式 */
	DMA_InitStructure.DMA_MemoryInc 						= 	DMA_MemoryInc_Enable;				/* 存储器增量模式 */
	DMA_InitStructure.DMA_PeripheralDataSize 				= 	DMA_PeripheralDataSize_HalfWord;	/* 外设数据长度16位 */
	DMA_InitStructure.DMA_MemoryDataSize 					= 	DMA_MemoryDataSize_HalfWord;		/* 存储器数据长度16位 */ 
	DMA_InitStructure.DMA_Mode 								= 	DMA_Mode_Circular;					/* 使用循环模式 */ 
	DMA_InitStructure.DMA_Priority 							= 	DMA_Priority_Medium;				/* 中等优先级 */
	DMA_InitStructure.DMA_FIFOMode 							= 	DMA_FIFOMode_Disable; 				/* 不使用FIFO模式 */        
	DMA_InitStructure.DMA_FIFOThreshold 					= 	DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst 						= 	DMA_MemoryBurst_Single;				/* 外设突发单次传输 */
	DMA_InitStructure.DMA_PeripheralBurst 					= 	DMA_PeripheralBurst_Single;			/* 存储器突发单次传输 */
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);														/* 初始化DMA Stream */
	
	DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t)dma_buffer1, DMA_Memory_0);					/* 双缓冲模式配置 */
	DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);													/* 双缓冲模式开启 */
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);													/* 开启传输完成中断 */
	
	NVIC_InitStructure.NVIC_IRQChannel 						= 	DMA1_Stream3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	=	1;								/* 抢占优先级0 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 	0;								/* 子优先级1 */
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= 	ENABLE;							/* 使能外部中断通道 */
	NVIC_Init(&NVIC_InitStructure);	
}

/* 
 * i2s dma回调函数指针
 */
void (*i2s_dma_tx_callback)(void);
void (*i2s_dma_rx_callback)(void);

/* 开启DMA TX传输,开始播放 */
void i2s_dma_tx_start(void) 
{
	DMA_Cmd(DMA1_Stream4, ENABLE);		
}

/* 关闭DMA,结束播放 */
void i2s_dma_tx_stop(void)
{
	DMA_Cmd(DMA1_Stream4, DISABLE);		
}

/* 开启DMA RX传输,开始录音 */
void i2s_dma_rx_start(void)
{
	DMA_Cmd(DMA1_Stream3, ENABLE);		
}

/* 关闭DMA,结束录音 */
void i2s_dma_rx_stop(void)
{
	DMA_Cmd(DMA1_Stream3, DISABLE);		
}

/*
 *	DMA1_Stream4中断服务函数, 放音
*/
void DMA1_Stream4_IRQHandler(void)
{
	rt_interrupt_enter();
	if (DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4) == SET)	{		/* DMA1_Stream4, 传输完成标志 */
		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
		i2s_dma_tx_callback();										/* 执行回调函数, 读取数据等操作在这里面处理 */ 
	} 
	rt_interrupt_leave();
}

/*
 *	DMA1_Stream3中断服务函数, 录音
*/
void DMA1_Stream3_IRQHandler(void)
{      
	rt_interrupt_enter();
	if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) == SET) {		/* DMA1_Stream3, 传输完成标志 */
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);			/* 清除传输完成中断 */
		i2s_dma_rx_callback();										/* 执行回调函数, 读取数据等操作在这里面处理 */
	}  
	rt_interrupt_leave();	
}

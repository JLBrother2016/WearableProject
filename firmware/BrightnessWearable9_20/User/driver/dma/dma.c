#include "dma.h"
#include "config.h"


/*
 * ����DMAΪTXģʽ�� �Ӵ洢��������
 * DMA1_Stream4 
*/
void i2s_dma_set_tx_mode(uint8_t *dma_buffer0, uint8_t *dma_buffer1, uint16_t dma_buffer_size)
{
	NVIC_InitTypeDef   										NVIC_InitStructure;
	DMA_InitTypeDef  										DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);					/* DMA1ʱ��ʹ��  */
	DMA_DeInit(DMA1_Stream4);												
	while (DMA_GetCmdStatus(DMA1_Stream4) != DISABLE);						/* �ȴ�DMA1_Stream4 ������ */
//	DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_FEIF4 | DMA_IT_DMEIF4 | DMA_IT_TEIF4 | DMA_IT_HTIF4 | DMA_IT_TCIF4);			/* ���DMA1_Stream4�������жϱ�־ */
	
	/* ����DMA Stream */
	DMA_InitStructure.DMA_Channel 							= 	DMA_Channel_0;  				/* ͨ��0 SPI2_TXͨ�� */
	DMA_InitStructure.DMA_PeripheralBaseAddr 				= 	(uint32_t)&SPI2->DR;			/* �����ַΪ:(u32)&SPI2->DR */
	DMA_InitStructure.DMA_Memory0BaseAddr 					= 	(uint32_t)dma_buffer0;			/* DMA �洢��0��ַ */
	DMA_InitStructure.DMA_DIR 								= 	DMA_DIR_MemoryToPeripheral;		/* �洢��������ģʽ */
	DMA_InitStructure.DMA_BufferSize 						= 	dma_buffer_size;				/* ���ݴ�����(�ֽ�)  */
	DMA_InitStructure.DMA_PeripheralInc 					= 	DMA_PeripheralInc_Disable;		/* ���������ģʽ */
	DMA_InitStructure.DMA_MemoryInc 						= 	DMA_MemoryInc_Enable;			/* �洢������ģʽ */
	DMA_InitStructure.DMA_PeripheralDataSize 				= 	DMA_PeripheralDataSize_HalfWord;/* �������ݳ���16λ */
	DMA_InitStructure.DMA_MemoryDataSize 					= 	DMA_MemoryDataSize_HalfWord;	/* �洢�����ݳ���16λ  */
	DMA_InitStructure.DMA_Mode 								= 	DMA_Mode_Circular;				/* ʹ��ѭ��ģʽ */
	DMA_InitStructure.DMA_Priority 							= 	DMA_Priority_High;				/* �����ȼ� */
	DMA_InitStructure.DMA_FIFOMode 							= 	DMA_FIFOMode_Disable; 			/* ��ʹ��FIFOģʽ */        
	DMA_InitStructure.DMA_FIFOThreshold 					= 	DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst 						= 	DMA_MemoryBurst_Single;			/* ����ͻ�����δ��� */
	DMA_InitStructure.DMA_PeripheralBurst 					= 	DMA_PeripheralBurst_Single;		/* �洢��ͻ�����δ��� */
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);													/* ��ʼ��DMA Stream */
	
	DMA_DoubleBufferModeConfig(DMA1_Stream4, (uint32_t)dma_buffer1, DMA_Memory_0);				/* ˫����ģʽ���� */
	DMA_DoubleBufferModeCmd(DMA1_Stream4, ENABLE);												/* ˫����ģʽ���� */
	DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);												/* ������������ж� */
	
	NVIC_InitStructure.NVIC_IRQChannel 						= 	DMA1_Stream4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	= 	2;								/* ��ռ���ȼ�0 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 	0;								/* �����ȼ�0 */
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= 	ENABLE;							/* ʹ���ⲿ�ж�ͨ�� */
	NVIC_Init(&NVIC_InitStructure);		
}


/*
 * ����DMAΪRXģʽ�� �����赽�洢�� 
 * ���� DMA1_Stream3
*/
void i2s_dma_set_rx_mode(uint8_t *dma_buffer0, uint8_t *dma_buffer1, uint16_t dma_buffer_size)
{
	NVIC_InitTypeDef   										NVIC_InitStructure;
	DMA_InitTypeDef  										DMA_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);					/* DMA1ʱ��ʹ��  */
	DMA_DeInit(DMA1_Stream3);
	while (DMA_GetCmdStatus(DMA1_Stream3) != DISABLE);						/* �ȴ�DMA1_Stream4 ������ */
	DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_FEIF3 | DMA_IT_DMEIF3 | DMA_IT_TEIF3 | DMA_IT_HTIF3 | DMA_IT_TCIF3);			/* ���DMA1_Stream4�������жϱ�־ */
	
	/* ���� DMA Stream */
	DMA_InitStructure.DMA_Channel 							= 	DMA_Channel_3;  					/* ͨ��3 I2S2ext_RXͨ�� */
	DMA_InitStructure.DMA_PeripheralBaseAddr 				= 	(uint32_t)&I2S2ext->DR;			/* �����ַΪ:(u32)&I2S2ext->DR>DR ʵ�ʵ�ַΪ 0x4000340C*/
	DMA_InitStructure.DMA_Memory0BaseAddr 					= 	(uint32_t)dma_buffer0;				/* DMA �洢��0��ַ */
	DMA_InitStructure.DMA_DIR 								= 	DMA_DIR_PeripheralToMemory;			/* ���赽�洢��ģʽ */
	DMA_InitStructure.DMA_BufferSize 						= 	dma_buffer_size;					/* ���ݴ��������ֽڣ� */
	DMA_InitStructure.DMA_PeripheralInc 					= 	DMA_PeripheralInc_Disable;			/* ���������ģʽ */
	DMA_InitStructure.DMA_MemoryInc 						= 	DMA_MemoryInc_Enable;				/* �洢������ģʽ */
	DMA_InitStructure.DMA_PeripheralDataSize 				= 	DMA_PeripheralDataSize_HalfWord;	/* �������ݳ���16λ */
	DMA_InitStructure.DMA_MemoryDataSize 					= 	DMA_MemoryDataSize_HalfWord;		/* �洢�����ݳ���16λ */ 
	DMA_InitStructure.DMA_Mode 								= 	DMA_Mode_Circular;					/* ʹ��ѭ��ģʽ */ 
	DMA_InitStructure.DMA_Priority 							= 	DMA_Priority_Medium;				/* �е����ȼ� */
	DMA_InitStructure.DMA_FIFOMode 							= 	DMA_FIFOMode_Disable; 				/* ��ʹ��FIFOģʽ */        
	DMA_InitStructure.DMA_FIFOThreshold 					= 	DMA_FIFOThreshold_1QuarterFull;
	DMA_InitStructure.DMA_MemoryBurst 						= 	DMA_MemoryBurst_Single;				/* ����ͻ�����δ��� */
	DMA_InitStructure.DMA_PeripheralBurst 					= 	DMA_PeripheralBurst_Single;			/* �洢��ͻ�����δ��� */
	DMA_Init(DMA1_Stream3, &DMA_InitStructure);														/* ��ʼ��DMA Stream */
	
	DMA_DoubleBufferModeConfig(DMA1_Stream3, (uint32_t)dma_buffer1, DMA_Memory_0);					/* ˫����ģʽ���� */
	DMA_DoubleBufferModeCmd(DMA1_Stream3, ENABLE);													/* ˫����ģʽ���� */
	DMA_ITConfig(DMA1_Stream3, DMA_IT_TC, ENABLE);													/* ������������ж� */
	
	NVIC_InitStructure.NVIC_IRQChannel 						= 	DMA1_Stream3_IRQn; 
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority 	=	1;								/* ��ռ���ȼ�0 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority 			= 	0;								/* �����ȼ�1 */
	NVIC_InitStructure.NVIC_IRQChannelCmd 					= 	ENABLE;							/* ʹ���ⲿ�ж�ͨ�� */
	NVIC_Init(&NVIC_InitStructure);	
}

/* 
 * i2s dma�ص�����ָ��
 */
void (*i2s_dma_tx_callback)(void);
void (*i2s_dma_rx_callback)(void);

/* ����DMA TX����,��ʼ���� */
void i2s_dma_tx_start(void) 
{
	DMA_Cmd(DMA1_Stream4, ENABLE);		
}

/* �ر�DMA,�������� */
void i2s_dma_tx_stop(void)
{
	DMA_Cmd(DMA1_Stream4, DISABLE);		
}

/* ����DMA RX����,��ʼ¼�� */
void i2s_dma_rx_start(void)
{
	DMA_Cmd(DMA1_Stream3, ENABLE);		
}

/* �ر�DMA,����¼�� */
void i2s_dma_rx_stop(void)
{
	DMA_Cmd(DMA1_Stream3, DISABLE);		
}

/*
 *	DMA1_Stream4�жϷ�����, ����
*/
void DMA1_Stream4_IRQHandler(void)
{
	rt_interrupt_enter();
	if (DMA_GetITStatus(DMA1_Stream4, DMA_IT_TCIF4) == SET)	{		/* DMA1_Stream4, ������ɱ�־ */
		DMA_ClearITPendingBit(DMA1_Stream4, DMA_IT_TCIF4);
		i2s_dma_tx_callback();										/* ִ�лص�����, ��ȡ���ݵȲ����������洦�� */ 
	} 
	rt_interrupt_leave();
}

/*
 *	DMA1_Stream3�жϷ�����, ¼��
*/
void DMA1_Stream3_IRQHandler(void)
{      
	rt_interrupt_enter();
	if(DMA_GetITStatus(DMA1_Stream3, DMA_IT_TCIF3) == SET) {		/* DMA1_Stream3, ������ɱ�־ */
		DMA_ClearITPendingBit(DMA1_Stream3, DMA_IT_TCIF3);			/* �����������ж� */
		i2s_dma_rx_callback();										/* ִ�лص�����, ��ȡ���ݵȲ����������洦�� */
	}  
	rt_interrupt_leave();	
}

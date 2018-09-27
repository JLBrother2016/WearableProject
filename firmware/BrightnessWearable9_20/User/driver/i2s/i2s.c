#include "i2s.h"
#include "config.h"
#include "dma.h"


/* �������ò����ʵ����� */
#define PSC_NUM 	11
static const uint16_t i2s_psc_tbl[PSC_NUM][5] = {
	
/* ������ PLLI2SN PLLI2SR I2SDIV ODD */	
	{ 800,  256,      5, 	12,   1},		/* 8Khz������ */
	{1102,  429,      4, 	19,   0},		/* 11.025Khz������ */ 
	{1600,  213,      2, 	13,   0},		/* 16Khz������ */
	{2205,  429,      4,  	 9,   1},		/* 22.05Khz������ */
	{3200,  213,      2,  	 6,   1},		/* 32Khz������ */
	{4410,  271, 	  2,  	 6,   0},		/* 44.1Khz������ */
	{4800,  258, 	  3,  	 3,   1},		/* 48Khz������ */
	{8820,  316, 	  2,  	 3,   1},		/* 88.2Khz������ */
	{9600,  344, 	  2,  	 3,   1},  		/* 96Khz������ */
	{17640, 361, 	  2,  	 2,   0},  		/* 176.4Khz������ */
	{19200, 393, 	  2,  	 2,   0},  		/* 192Khz������ */
};


/*
  *  +-------------------------------------------------------------------------+			
						  I2S �ܽ���������
					SCL 	    		 	PB6
					SDA						PB7					     
					I2S_WS					PB12
					I2SCK					PB13
					I2Sext_SD				PC2
					I2S_SD					PC3
					I2S_MCK					PC6					  												
  *	 +-------------------------------------------------------------------------+
*/
void i2s_gpio_config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);	/* ʹ������GPIOB,GPIOCʱ�� */
	
	/* PB12/13 ���ù������ */
	GPIO_InitStructure.GPIO_Pin 			= GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF;							/* ���ù��� */
	GPIO_InitStructure.GPIO_OType 			= GPIO_OType_PP;						/* ���� */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_100MHz;					/* 100MHz */
	GPIO_InitStructure.GPIO_PuPd 			= GPIO_PuPd_UP;							/* ���� */
	GPIO_Init(GPIOB, &GPIO_InitStructure);											/* ��ʼ�� */
	
	/* PC2/PC3/PC6���ù������ */
	GPIO_InitStructure.GPIO_Pin 			= GPIO_Pin_2 | GPIO_Pin_3|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF;							/* ���ù��� */
	GPIO_InitStructure.GPIO_OType 			= GPIO_OType_PP;						/* ���� */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_100MHz;					/* 100MHz */
	GPIO_InitStructure.GPIO_PuPd 			= GPIO_PuPd_UP;							/* ���� */
	GPIO_Init(GPIOC, &GPIO_InitStructure);											/* ��ʼ�� */
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2); 						/* PB12,AF5  I2S_LRCK */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);						/* PB13,AF5  I2S_SCLK */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3,  GPIO_AF_SPI2);						/* PC3 ,AF5  I2S_DACDATA */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6,  GPIO_AF_SPI2);						/* PC6 ,AF5  I2S_MCK */
//	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2,  GPIO_AF6_SPI2);						/* PC2 ,AF6  I2S_ADCDATA  I2S2ext_SD��AF6!!! */	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2,  0x06);
}

static void i2s_clock_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	/* ʹ��I2S2ʱ�� */
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);	/* ��λSPI2 */
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);	/* ������λ */
}


/* 
 * ������ʱ������Ϊtxģʽ, SPI2 
 * ���� i2s_data_format�� 
		I2S_DataFormat_16b 
		I2S_DataFormat_16bextended
		I2S_DataFormat_24b
		I2S_DataFormat_32b
*/
void i2s_set_tx_mode(uint16_t i2s_data_format)
{
	I2S_InitTypeDef 						I2S_InitStructure;
	
	i2s_clock_init();														/* ʹ��I2Sʱ�� */
	I2S_InitStructure.I2S_Mode 			= 	I2S_Mode_MasterTx;				/* I2S ����ģʽ */
	I2S_InitStructure.I2S_Standard		= 	I2S_Standard_Phillips;			/* Phillips��׼ */
//	I2S_InitStructure.I2S_DataFormat	=	I2S_DataFormat_16b;
	I2S_InitStructure.I2S_DataFormat	=	i2s_data_format;				/* I2S ���ݳ��� */
	I2S_InitStructure.I2S_MCLKOutput	=	I2S_MCLKOutput_Disable;			/* ��ʱ�������ֹ */
	I2S_InitStructure.I2S_AudioFreq		=	I2S_AudioFreq_Default;			/* I2S Ƶ�� */
	I2S_InitStructure.I2S_CPOL			=	I2S_CPOL_Low;					/* ����״̬ʱ�ӵ�ƽ */
	
	I2S_Init(SPI2, &I2S_InitStructure);										/* ��ʼ��I2S */
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);						/* SPI2 TX DMA ����ʹ�� */
	I2S_Cmd(SPI2, ENABLE);													/* SPI2 I2S ENʹ�� */
}

/*
 *  ¼����ʱ������Ϊrxģʽ, I2S2ext
*/
void i2s_set_rx_mode(uint16_t i2s_data_format)
{
	I2S_InitTypeDef 							I2S2ext_InitStructure;
	
	I2S2ext_InitStructure.I2S_Mode			=	I2S_Mode_SlaveRx ^ (1 << 8);    	/* I2S �ӻ�ģʽ */
	I2S2ext_InitStructure.I2S_Standard		=	I2S_Standard_Phillips;				/* Phillips��׼ */
//	I2S2ext_InitStructure.I2S_DataFormat	=	I2S_DataFormat_16b;
	I2S2ext_InitStructure.I2S_DataFormat	=	i2s_data_format;					/* I2S ���ݳ��� */
	I2S2ext_InitStructure.I2S_MCLKOutput	=	I2S_MCLKOutput_Disable;				/* ��ʱ�������ֹ */
	I2S2ext_InitStructure.I2S_AudioFreq		=	I2S_AudioFreq_Default;				/* I2S Ƶ�� */
	I2S2ext_InitStructure.I2S_CPOL			=	I2S_CPOL_Low;						/* ����״̬ʱ�ӵ�ƽ */
	
	I2S_FullDuplexConfig(I2S2ext, &I2S2ext_InitStructure);							/* ��ʼ��I2S2ext���� */
	SPI_I2S_DMACmd(I2S2ext, SPI_I2S_DMAReq_Rx, ENABLE);								/* I2S2ext RX DMA����ʹ�� */
	I2S_Cmd(I2S2ext, ENABLE);														/* I2S2ext I2S ENʹ�� */
}



/*
 * ���ò�����
*/
int32_t i2s_set_sample_rate(uint32_t sr)
{
	uint8_t i;
	uint32_t tempreg = 0;
	uint32_t sr_tmp = sr / 10;						/* ��С10�� */			
	
	for (i = 0; i < PSC_NUM; i++) {
		if (sr_tmp == i2s_psc_tbl[i][0]) 			/* �ڱ�����ҵ��˲����� */
			break;
	}
	
	RCC_PLLI2SCmd(DISABLE);							/* �ȹر�PLLI2S */
	if (i == PSC_NUM) {								/* �ѱ���Ҳ�Ҳ��� */
		err("Ԥ�������������û���ҵ��ò�����: %d", sr);
		return -1;
	}
	
	/* ����I2SxCLK��Ƶ��(x=2)  ����PLLI2SN PLLI2SR */
	RCC_PLLI2SConfig((uint32_t)i2s_psc_tbl[i][1], (uint32_t)i2s_psc_tbl[i][2]);
	RCC->CR |= 1 << 26;					/* ����I2Sʱ�� */
	while ((RCC->CR & 1 << 27) == 0);	/* �ȴ�I2Sʱ�ӿ����ɹ�. */
	
	tempreg = i2s_psc_tbl[i][3] << 0;	/* ����I2SDIV */
	tempreg |= i2s_psc_tbl[i][4] << 8;	/* ����ODDλ */
	tempreg |= 1 << 9;					/* ʹ��MCKOEλ,���MCK */
	SPI2->I2SPR = tempreg;				/* ����I2SPR�Ĵ���(SPI_I2SԤ��Ƶ�Ĵ���) */
	
	return 0;
}



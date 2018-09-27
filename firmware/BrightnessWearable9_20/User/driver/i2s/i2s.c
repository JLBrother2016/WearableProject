#include "i2s.h"
#include "config.h"
#include "dma.h"


/* 用来设置采样率的数组 */
#define PSC_NUM 	11
static const uint16_t i2s_psc_tbl[PSC_NUM][5] = {
	
/* 采样率 PLLI2SN PLLI2SR I2SDIV ODD */	
	{ 800,  256,      5, 	12,   1},		/* 8Khz采样率 */
	{1102,  429,      4, 	19,   0},		/* 11.025Khz采样率 */ 
	{1600,  213,      2, 	13,   0},		/* 16Khz采样率 */
	{2205,  429,      4,  	 9,   1},		/* 22.05Khz采样率 */
	{3200,  213,      2,  	 6,   1},		/* 32Khz采样率 */
	{4410,  271, 	  2,  	 6,   0},		/* 44.1Khz采样率 */
	{4800,  258, 	  3,  	 3,   1},		/* 48Khz采样率 */
	{8820,  316, 	  2,  	 3,   1},		/* 88.2Khz采样率 */
	{9600,  344, 	  2,  	 3,   1},  		/* 96Khz采样率 */
	{17640, 361, 	  2,  	 2,   0},  		/* 176.4Khz采样率 */
	{19200, 393, 	  2,  	 2,   0},  		/* 192Khz采样率 */
};


/*
  *  +-------------------------------------------------------------------------+			
						  I2S 管脚配置如下
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
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);	/* 使能外设GPIOB,GPIOC时钟 */
	
	/* PB12/13 复用功能输出 */
	GPIO_InitStructure.GPIO_Pin 			= GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF;							/* 复用功能 */
	GPIO_InitStructure.GPIO_OType 			= GPIO_OType_PP;						/* 推挽 */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_100MHz;					/* 100MHz */
	GPIO_InitStructure.GPIO_PuPd 			= GPIO_PuPd_UP;							/* 上拉 */
	GPIO_Init(GPIOB, &GPIO_InitStructure);											/* 初始化 */
	
	/* PC2/PC3/PC6复用功能输出 */
	GPIO_InitStructure.GPIO_Pin 			= GPIO_Pin_2 | GPIO_Pin_3|GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode 			= GPIO_Mode_AF;							/* 复用功能 */
	GPIO_InitStructure.GPIO_OType 			= GPIO_OType_PP;						/* 推挽 */
	GPIO_InitStructure.GPIO_Speed 			= GPIO_Speed_100MHz;					/* 100MHz */
	GPIO_InitStructure.GPIO_PuPd 			= GPIO_PuPd_UP;							/* 上拉 */
	GPIO_Init(GPIOC, &GPIO_InitStructure);											/* 初始化 */
	
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_SPI2); 						/* PB12,AF5  I2S_LRCK */
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);						/* PB13,AF5  I2S_SCLK */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource3,  GPIO_AF_SPI2);						/* PC3 ,AF5  I2S_DACDATA */
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6,  GPIO_AF_SPI2);						/* PC6 ,AF5  I2S_MCK */
//	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2,  GPIO_AF6_SPI2);						/* PC2 ,AF6  I2S_ADCDATA  I2S2ext_SD是AF6!!! */	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource2,  0x06);
}

static void i2s_clock_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);	/* 使能I2S2时钟 */
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, ENABLE);	/* 复位SPI2 */
	RCC_APB1PeriphResetCmd(RCC_APB1Periph_SPI2, DISABLE);	/* 结束复位 */
}


/* 
 * 放音的时候设置为tx模式, SPI2 
 * 参数 i2s_data_format： 
		I2S_DataFormat_16b 
		I2S_DataFormat_16bextended
		I2S_DataFormat_24b
		I2S_DataFormat_32b
*/
void i2s_set_tx_mode(uint16_t i2s_data_format)
{
	I2S_InitTypeDef 						I2S_InitStructure;
	
	i2s_clock_init();														/* 使能I2S时钟 */
	I2S_InitStructure.I2S_Mode 			= 	I2S_Mode_MasterTx;				/* I2S 主机模式 */
	I2S_InitStructure.I2S_Standard		= 	I2S_Standard_Phillips;			/* Phillips标准 */
//	I2S_InitStructure.I2S_DataFormat	=	I2S_DataFormat_16b;
	I2S_InitStructure.I2S_DataFormat	=	i2s_data_format;				/* I2S 数据长度 */
	I2S_InitStructure.I2S_MCLKOutput	=	I2S_MCLKOutput_Disable;			/* 主时钟输出禁止 */
	I2S_InitStructure.I2S_AudioFreq		=	I2S_AudioFreq_Default;			/* I2S 频率 */
	I2S_InitStructure.I2S_CPOL			=	I2S_CPOL_Low;					/* 空闲状态时钟电平 */
	
	I2S_Init(SPI2, &I2S_InitStructure);										/* 初始化I2S */
	SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);						/* SPI2 TX DMA 请求使能 */
	I2S_Cmd(SPI2, ENABLE);													/* SPI2 I2S EN使能 */
}

/*
 *  录音的时候设置为rx模式, I2S2ext
*/
void i2s_set_rx_mode(uint16_t i2s_data_format)
{
	I2S_InitTypeDef 							I2S2ext_InitStructure;
	
	I2S2ext_InitStructure.I2S_Mode			=	I2S_Mode_SlaveRx ^ (1 << 8);    	/* I2S 从机模式 */
	I2S2ext_InitStructure.I2S_Standard		=	I2S_Standard_Phillips;				/* Phillips标准 */
//	I2S2ext_InitStructure.I2S_DataFormat	=	I2S_DataFormat_16b;
	I2S2ext_InitStructure.I2S_DataFormat	=	i2s_data_format;					/* I2S 数据长度 */
	I2S2ext_InitStructure.I2S_MCLKOutput	=	I2S_MCLKOutput_Disable;				/* 主时钟输出禁止 */
	I2S2ext_InitStructure.I2S_AudioFreq		=	I2S_AudioFreq_Default;				/* I2S 频率 */
	I2S2ext_InitStructure.I2S_CPOL			=	I2S_CPOL_Low;						/* 空闲状态时钟电平 */
	
	I2S_FullDuplexConfig(I2S2ext, &I2S2ext_InitStructure);							/* 初始化I2S2ext配置 */
	SPI_I2S_DMACmd(I2S2ext, SPI_I2S_DMAReq_Rx, ENABLE);								/* I2S2ext RX DMA请求使能 */
	I2S_Cmd(I2S2ext, ENABLE);														/* I2S2ext I2S EN使能 */
}



/*
 * 设置采样率
*/
int32_t i2s_set_sample_rate(uint32_t sr)
{
	uint8_t i;
	uint32_t tempreg = 0;
	uint32_t sr_tmp = sr / 10;						/* 缩小10倍 */			
	
	for (i = 0; i < PSC_NUM; i++) {
		if (sr_tmp == i2s_psc_tbl[i][0]) 			/* 在表格中找到了采样率 */
			break;
	}
	
	RCC_PLLI2SCmd(DISABLE);							/* 先关闭PLLI2S */
	if (i == PSC_NUM) {								/* 搜遍了也找不到 */
		err("预设采样率数组中没有找到该采样率: %d", sr);
		return -1;
	}
	
	/* 设置I2SxCLK的频率(x=2)  设置PLLI2SN PLLI2SR */
	RCC_PLLI2SConfig((uint32_t)i2s_psc_tbl[i][1], (uint32_t)i2s_psc_tbl[i][2]);
	RCC->CR |= 1 << 26;					/* 开启I2S时钟 */
	while ((RCC->CR & 1 << 27) == 0);	/* 等待I2S时钟开启成功. */
	
	tempreg = i2s_psc_tbl[i][3] << 0;	/* 设置I2SDIV */
	tempreg |= i2s_psc_tbl[i][4] << 8;	/* 设置ODD位 */
	tempreg |= 1 << 9;					/* 使能MCKOE位,输出MCK */
	SPI2->I2SPR = tempreg;				/* 设置I2SPR寄存器(SPI_I2S预分频寄存器) */
	
	return 0;
}



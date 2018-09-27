#include "oled.h"
#include "fonts.h"
#include "config.h"

/* global variable. */
struct device_oled device_oled;

/* -----------------------OLED12864控制引脚定义 -------------------------------*/
	/* 
	 * OLED12864 接口引脚配置如下
	 * 		DIN（MOSI） = PA7			配置为SPI
	 * 		CLK(SCK)    = PA5			配置为SPI
	 *					= PB14 			配置为SPI
	 *		CS			= PC4			配置为推挽
	 *		D/C         = PB0			配置为推挽
	 * 		RES 		= PC5			配置为推挽
	 *
	 * STM32硬件SPI接口 = SPI1
	*/
#define RCC_OLED_CTL_PORT		RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC
#define OLED_CTL_PORT_CS		GPIOC
#define OLED_CS_GPIO 			GPIOC
#define OLED_CS_PIN 			GPIO_Pin_4

#define OLED_CTL_PORT_DC		GPIOB
#define OLED_DC_GPIO 			GPIOB
#define OLED_DC_PIN 			GPIO_Pin_0

#define OLED_CTL_PORT_RES		GPIOC
#define OLED_RES_GPIO 			GPIOC
#define OLED_RES_PIN			GPIO_Pin_5

/* 片选CS置低选中  -----------------------------------------------------------*/
#define OLED_CS_LOW()       	OLED_CS_GPIO->BSRRH = OLED_CS_PIN
/* 片选CS置高不选中 ----------------------------------------------------------*/
#define OLED_CS_HIGH()      	OLED_CS_GPIO->BSRRL = OLED_CS_PIN
/* 复位RES置低复位 -----------------------------------------------------------*/
#define OLED_RES_LOW()			OLED_RES_GPIO->BSRRH = OLED_RES_PIN
/* 复位RES置高不复位 ---------------------------------------------------------*/
#define OLED_RES_HIGH() 		OLED_RES_GPIO->BSRRL = OLED_RES_PIN
/* 数据/命令DC置低传输命令 ---------------------------------------------------*/
#define OLED_DC_LOW()			OLED_DC_GPIO->BSRRH = OLED_DC_PIN
/* 数据/命令DC置高传输数据 ---------------------------------------------------*/
#define OLED_DC_HIGH() 			OLED_DC_GPIO->BSRRL = OLED_DC_PIN

/* 显示宽度和高度定义 --------------------------------------------------------*/
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

/* 选择列开始的地址 */
#define __SET_COL_START_ADDR() do \
								{ \
									oled_write_byte(0x02, OLED_CMD); \
									oled_write_byte(0x10, OLED_CMD); \
								} while(0)
								
								
/**
  ******************************************************************************
  * OLED12864显存，存放格式如下：
  *  [Page0] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page1] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page2] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page3] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page4] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page5] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page6] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page7] Byte0 Byte1 ---------------------------------------Byte127
  *
  *  OLED_GRAM[128][64]中的128代表列数（X坐标），8代表页，每页纵向8点，共64行（Y坐标）
  *  从高到低对应行数从小到大。
  *  一个通用点（x, y）置1表达式为：
  * 		OLED_GRAM[x][7 - y/8] |= 1 << (7 - y%8)
  *  x范围0-127； y范围0-63
  ******************************************************************************
  */								
static uint8_t OLED_GRAM[128][8];


/* ---------------------------------Private function prototypes -----------------------------------------------*/
static void oled_write_byte(uint8_t _chData, CMD_DAT_t _ch);
static void oled_display_char(uint8_t _chXpos, uint8_t _chYpos, uint8_t _chChr, uint8_t _chSize, DISPLAY_MODE_t _chMode);

int32_t stm32_hw_oled_spi_init(void)
{	
	GPIO_InitTypeDef 		GPIO_InitStructure;
	SPI_InitTypeDef  		SPI_InitStructure;
	
	/* 使能GPIO 时钟 */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* 配置 SCK, MISO 、 MOSI 为复用功能 */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	/* 引脚初始化 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* 使能OLED控制口的时钟 */
	RCC_AHB1PeriphClockCmd(RCC_OLED_CTL_PORT, ENABLE);
	
	/* 片选不选中 */
	OLED_CS_HIGH();
	
	/* CS RES DC配置为推挽输出 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = OLED_CS_PIN;
	GPIO_Init(OLED_CTL_PORT_CS, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = OLED_DC_PIN;
	GPIO_Init(OLED_CTL_PORT_DC, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = OLED_RES_PIN;
	GPIO_Init(OLED_CTL_PORT_RES, &GPIO_InitStructure);
	
	/* 打开SPI1时钟 */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* 配置SPI1硬件参数 */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 数据方向：2线全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						/* SPI工作模式 ：主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					/* 数据位长度 ： 8位 */
	
	/* SPI_CPOL和SPI_CPHA结合使用决定时钟和数据采样点的相位关系、
	   本例配置: 总线空闲是高电平,第2个边沿（上升沿采样数据)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;							/* 时钟上升沿采样数据 */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;						/* 时钟的第2个边沿采样数据 */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							/* 片选控制方式：软件控制 */

	/* 设置波特率预分频系数 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;					/* 数据位传输次序：高位先传 */
	SPI_InitStructure.SPI_CRCPolynomial = 7;							/* CRC多项式寄存器，复位后为7。本例程不用 */
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, DISABLE);												/* 先禁止SPI  */
	SPI_Cmd(SPI1, ENABLE);												/* 使能SPI  */
	
	return 0;
}

int32_t oled_init(device_oled_t dev)
{ 
	OLED_CS_HIGH();
	OLED_DC_LOW();
	OLED_RES_HIGH();
	
	oled_write_byte(0xAE, OLED_CMD);		/* 关闭OLED面板显示（休眠） */
	oled_write_byte(0x00, OLED_CMD);		/* 设置列地址低4bit */
	oled_write_byte(0x10, OLED_CMD);		/* 设置列地址高4bit */
	oled_write_byte(0x40, OLED_CMD);		/* 设置起始行地址（低5bit 0-63）， 硬件相关 */
	oled_write_byte(0x81, OLED_CMD);		/* 设置对比度命令（双字节命令），第1个字节是命令，第2个字节是对比度参数0-255 */
	oled_write_byte(0xCF, OLED_CMD);		/* 设置对比度参数，缺省CF */
	oled_write_byte(0xA1, OLED_CMD);		/* A0 ：列地址0映射到SEG0; A1 ：列地址131映射到SEG0 */
	oled_write_byte(0xC0, OLED_CMD);		/* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
	oled_write_byte(0xA6, OLED_CMD);		/* A6 : 设置正常显示模式; A7 : 设置为反显模式 */
	oled_write_byte(0xA8, OLED_CMD);		/* 设置COM路数 */
	oled_write_byte(0x3f, OLED_CMD);		/* 1 ->（63+1）路 */
	oled_write_byte(0xD3, OLED_CMD);		/* 设置显示偏移（双字节命令）*/
	oled_write_byte(0x00, OLED_CMD);		/* 无偏移 */
	oled_write_byte(0xd5, OLED_CMD);		/* 设置显示时钟分频系数/振荡频率 */
	oled_write_byte(0x80, OLED_CMD);		/* 设置分频系数,高4bit是分频系数，低4bit是振荡频率 */
	oled_write_byte(0xD9, OLED_CMD);		/* 设置预充电周期 */
	oled_write_byte(0xF1, OLED_CMD);		/* [3:0],PHASE 1; [7:4],PHASE 2; */
	oled_write_byte(0xDA, OLED_CMD);		/* 设置COM脚硬件接线方式 */
	oled_write_byte(0x12, OLED_CMD);
	oled_write_byte(0xDB, OLED_CMD);		/* 设置 vcomh 电压倍率 */
	oled_write_byte(0x40, OLED_CMD);		/* [6:4] 000 = 0.65 x VCC; 0.77 x VCC (RESET); 0.83 x VCC  */
	oled_write_byte(0x20, OLED_CMD);
	oled_write_byte(0x02, OLED_CMD);
	oled_write_byte(0x8D, OLED_CMD);		/* 设置充电泵（和下个命令结合使用） */
	oled_write_byte(0x14, OLED_CMD);		/* 0x14 使能充电泵， 0x10 是关闭 */
	oled_write_byte(0xA4, OLED_CMD);
	oled_write_byte(0xA6, OLED_CMD);
	dev->clear_screen();
	oled_write_byte(0xAF, OLED_CMD);		/* 打开OLED面板 */
	debug("OLED 初始化成功！\r\n");
	return 0;
}

/**
  ******************************************************************************
  * @brief   从SPI1发送一个字节，同时从MISO口线采样器件返回的数据
  * @param   _ch : 发送的字节值
  * @retval  从MISO口线采样器件返回的数据
  ******************************************************************************
  */
static uint8_t spi_send_byte(uint8_t _ch)
{
	/* 等待上个数据未发送完毕 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	/* 通过SPI硬件发送1个字节 */
	SPI_I2S_SendData(SPI1, _ch);
	
	/* 等待接收一个字节任务完成 */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* 返回从SPI总线读到的数据 */
	return SPI_I2S_ReceiveData(SPI1);
}

/**
  ******************************************************************************
  * @brief   向oled发送一个数据或命令
  * @param   _chData  发送的字节； 
  *  		 _chCMD 0 为传输命令 ， 1 为传输数据
  * @retval  None
  ******************************************************************************
  */
static void oled_write_byte(uint8_t _chData, CMD_DAT_t _ch)
{
	assert_param(IS_CMD_DAT_VAL(_ch));
	
	/* 选中OLED */
	OLED_CS_LOW();
	if(_ch == OLED_DAT)
	{
		OLED_DC_HIGH(); 		/* 传输数据 */
	}
	else if(_ch == OLED_CMD)
	{
		OLED_DC_LOW(); 			/* 传输命令 */
	}
	
	/* SPI1 发送一个字节 */
	spi_send_byte(_chData);
	
	OLED_CS_HIGH();			/* 不选中OLED */
}


/**
  ******************************************************************************
  * @brief   打开OLED显示
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_turn_on(void)
{
	oled_write_byte(0x8D, OLED_CMD);	/* 设置DCDC命令，设置充电泵（和下一个命令结合使用） */
	oled_write_byte(0x14, OLED_CMD);    /* 打开DCDC， 0x14是使能充电泵， 0x10是关闭 */
	oled_write_byte(0xAF, OLED_CMD);    /* 打开OLED 面板 */
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   关闭OLED显示
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_turn_off(void)
{
	oled_write_byte(0x8D, OLED_CMD);	/* 设置DCDC命令，设置充电泵（和下一个命令结合使用） */
	oled_write_byte(0x10, OLED_CMD); 	/* 打开DCDC， 0x14是使能充电泵， 0x10是关闭 */
	oled_write_byte(0xAF, OLED_CMD);	/* 关闭OLED 面板 */
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   设置显示方向
  * @param   _ucDir = 0表示正常方向， 1表示反转180度
  * @retval  None
  ******************************************************************************
  */
int32_t oled_set_dir(uint8_t _ucDir)
{
	if(_ucDir == 0)
	{
		oled_write_byte(0xA0, OLED_CMD);	/* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
		oled_write_byte(0xC0, OLED_CMD);    /* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
	}
	else
	{
		oled_write_byte(0xA1, OLED_CMD);	/* A0 ：列地址0映射到SEG0; A1 ：列地址127映射到SEG0 */
		oled_write_byte(0xC8, OLED_CMD);	/* C0 ：正常扫描,从COM0到COM63;  C8 : 反向扫描, 从 COM63至 COM0 */
	}
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   设置显示对比度
  * @param   _ucValue 对比度参数0-255
  * @retval  None
  ******************************************************************************
  */
int32_t oled_set_contrast(uint8_t _ucValue)
{
	oled_write_byte(0x81, OLED_CMD);
	oled_write_byte(_ucValue, OLED_CMD);
	return 0;
}



/**
  ******************************************************************************
  * @brief   Refresh the graphic ram 将OLED_GRAM[128][8]刷入 OLED里面
  *          该函数在SysTick中断里面每10ms被调用一次，也就是刷新频率为100HZ
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_refresh_gram(void)
{
	uint8_t i, j;
	
	for(i = 0; i < 8; i++) 				/* 遍历Page */
	{
		oled_write_byte(0xB0 + i, OLED_CMD);
		
		__SET_COL_START_ADDR();			/* 选择列地址，SH1106从COL2开始 */
		
		for(j = 0; j < 128; j++)		/* 遍历列 */
		{
			oled_write_byte(OLED_GRAM[j][i], OLED_DAT);
		}
	}
	
	return 0;
}

/**
  ******************************************************************************
  * @brief   clear the screen 清完屏后整个屏幕是黑色的，和没点亮一样
  * 		 但此时OLED是工作的
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_clear_screen(void)
{
	uint8_t i, j;
	
	for(i = 0; i < 8; i++)								/* 遍历Page */
	{
		oled_write_byte(0xB0 + i, OLED_CMD);
		
		__SET_COL_START_ADDR();
		
		for(j = 0; j < 128; j++)						/* 遍历列 */	
		{
			OLED_GRAM[j][i] = 0x00;
		}
	}
	
//	oled_refresh_gram();							/* 刷屏 */
	return 0;
}


/**
  ******************************************************************************
  * @brief   draw a point on the screen 这个是OLED显示的核心函数
  * @param   _chXpos specify the X position
  * 		 _chYpos specify the Y position
  *  		 _chPoint 1 填充 0 清空
  * @retval  None
  ******************************************************************************
  */
int32_t oled_draw_point(uint8_t _chXpos, uint8_t _chYpos, DISPLAY_MODE_t _chPoint)
{
	uint8_t chPos, chBx, chTemp;
	
	assert_param(IS_DISPLAY_MODE_VAL(_chPoint));
	
	/* 检查坐标的合理性128*64 */
	if(_chXpos > 127 || _chYpos > 63)
	{
		return -1;
	}
	
	chPos = 7 - _chYpos / 8;	/* 计算页地址中的行地址 */
	chBx = _chYpos % 8;        	/* 计算页地址 */
	chTemp = 1 << (7 - chBx);
	
	if(_chPoint)
	{
		OLED_GRAM[_chXpos][chPos] |= chTemp;
	}
	
	else
	{
		OLED_GRAM[_chXpos][chPos] &= ~chTemp;
	}
	
	return 0;
}


/**
  ******************************************************************************
  * @brief  Displays a character at the specified position    
  * @param  chXpos: Specifies the X position， 对于12864屏幕，范围【0-127】
  *         chYpos: Specifies the Y position， 对于12864平布，范围【0-63】
  *         chSize: 选择字体
  *         chMode 0 反白显示， 1 正常显示
  * @retval 
  ******************************************************************************
**/
static void oled_display_char(uint8_t _chXpos, uint8_t _chYpos, uint8_t _chChr, CHAR_SIZE_t _chSize, DISPLAY_MODE_t _chMode)
{      	
	uint8_t i, j;
	uint8_t chTemp, chYpos0 = _chYpos;
	
	_chChr = _chChr - ' ';				   
    for (i = 0; i < _chSize; i++) 
	{   
		if (_chSize == SIZE_12) 				/* 使用1206字体 */
		{
			if (_chMode == NORMAL) 
			{
				chTemp = chFont1206[_chChr][i];
			} 
			else 
			{
				chTemp = ~chFont1206[_chChr][i];
			}
		}
		
		else if(_chSize == SIZE_16)   		/* 使用1608字体 */
		{
			if (_chMode == NORMAL) 
			{
				chTemp = chFont1608[_chChr][i];
			} 
			else 
			{
				chTemp = ~chFont1608[_chChr][i];
			}
		}
		
		else
		{
			return;
		}
		
        for (j = 0; j < 8; j++) 
		{
			if (chTemp & 0x80) 
			{
				oled_draw_point(_chXpos, _chYpos, NORMAL);
			}
			
			else 
			{
				oled_draw_point(_chXpos, _chYpos, INVERSE);
			}
			
			chTemp <<= 1;
			_chYpos++;
			
			if ((_chYpos - chYpos0) == _chSize) 
			{
				_chYpos = chYpos0;
				_chXpos ++;
				break;
			}
		}  	 
    } 
}

static uint32_t o_pow(uint8_t m, uint8_t n)
{
	uint32_t result = 1;
	
	while(n --) 
	{	
		result *= m;  
	}
	
	return result;
}



/**
  ******************************************************************************
  * @brief  Displays a number at the specified position    
  *         
  * @param  chXpos: Specifies the X position
  *         chYpos: Specifies the Y position
  *         _chNum: 数值变量
  *         _chLen: 数字的位数
  *  		_chSize: 字体大小
  * @retval 
  ******************************************************************************
**/
int32_t oled_display_number(uint8_t _chXpos, uint8_t _chYpos, uint32_t _chNum, uint8_t _chLen, CHAR_SIZE_t _chSize)
{
	uint8_t i, chTemp, chShow;
	
//	assert_param(IS_CHAR_SIZE_VAL(_chSize));
	
	for(i = 0; i < _chLen; i++) 
	{
		chTemp = (_chNum / o_pow(10, _chLen - i - 1)) % 10;
		
		if(chShow == 0 && i < (_chLen - 1)) 
		{
			if(chTemp == 0) 
			{
				oled_display_char(_chXpos + (_chSize / 2) * i, _chYpos, ' ', _chSize, NORMAL);
				
				continue;
			} 
			else 
			{
				chShow = 1;
			}	 
		}
		
	 	oled_display_char(_chXpos + (_chSize / 2) * i, _chYpos, chTemp + '0', _chSize, NORMAL); 
	}
	
	return 0;
}

/**
  ******************************************************************************
  * @brief  Displays a string on the screen
  *         
  * @param  chXpos: Specifies the X position
  *         chYpos: Specifies the Y position
  *         pchString: Pointer to a string to display on the screen 
  *         chSize 字体大小
  * 		chMode 显示模式 0 反显， 1 正显
  * @retval  None
  ******************************************************************************
**/
//int32_t oled_display_string(uint8_t _chXpos, uint8_t _chYpos, const uint8_t *_pchString, CHAR_SIZE_t _chSize, DISPLAY_MODE_t _chMode)
int32_t oled_display_string(uint8_t _chXpos, uint8_t _chYpos, const char *_pchString, CHAR_SIZE_t _chSize, DISPLAY_MODE_t _chMode)
{
//	assert_param(IS_DISPLAY_MODE_VAL(_chMode));
	
    while (*_pchString != '\0') 
	{       
        if (_chXpos > (OLED_WIDTH - _chSize / 2)) 
		{
			_chXpos = 0;
			_chYpos += _chSize;
			
			if (_chYpos > (OLED_HEIGHT - _chSize)) 
			{
				_chYpos = _chXpos = 0;
				
				oled_clear_screen();
			}
		}
		
        oled_display_char(_chXpos, _chYpos, *_pchString, _chSize, _chMode);
		
        _chXpos += _chSize / 2;
		
        _pchString ++;
    }
	
	return 0;
}


/**
  ******************************************************************************
  * @brief  Displays a bitma on the screen
  *         
  * @param  chXpos: Specifies the X position
  *         chYpos: Specifies the Y position
  *         pchBmp: Pointer to a bitMap to display on the screen 
  *         
  * @retval  None
  ******************************************************************************
**/
int32_t oled_draw_bitmap(uint8_t _chXpos, uint8_t _chYpos, const uint8_t *_pchBmp, uint8_t _chWidth, uint8_t _chHeight)
{
	uint16_t i, j, byteWidth = (_chWidth + 7) / 8;
	
    for(j = 0; j < _chHeight; j ++)
	{
        for(i = 0; i < _chWidth; i ++ ) 
		{
            if(*(_pchBmp + j * byteWidth + i / 8) & (128 >> (i & 7))) 
			{
                oled_draw_point(_chXpos + i, _chYpos + j, NORMAL);
            }
        }
    }
	
	return 0;
}



/**
  ******************************************************************************
  * @brief  draw a line on the screen   采用Bresenham算法，在2点间画一条直线     
  * @param  _usX1, _usY1 ：起始点坐标
  *   		_usX2, _usY2 ：终止点坐标
  * @retval  None
  ******************************************************************************
**/
int32_t oled_draw_line(uint16_t _usX1, uint16_t _usY1, uint16_t _usX2, uint16_t _usY2)
{
	int32_t dx, dy;
	int32_t tx, ty;
	int32_t inc1, inc2;
	int32_t d, iTag;
	int32_t x, y;
	
	/* 采用 Bresenham 算法，在2点间画一条直线 */
	oled_draw_point(_usX1, _usY1, NORMAL);
	
	/* 如果两点重合，结束后面的动作。*/
	if((_usX1 == _usX2) && (_usY1 == _usY2))
		return -1;
	
	iTag = 0;
	/* dx = abs ( _usX2 - _usX1 ); */
	if(_usX2 >= _usX1)
		dx = _usX2 - _usX1;
	else
		dx = _usX1 - _usX2;
	
	/* dy = abs ( _usY2 - _usY1 ); */
	if (_usY2 >= _usY1)
		dy = _usY2 - _usY1;
	else
		dy = _usY1 - _usY2;
	
	if ( dx < dy ) {  /*如果dy为计长方向，则交换纵横坐标。*/
		uint16_t temp;
		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* 确定是增1还是减1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	
	while ( x != _usX2 ) {    /* 循环画点 */
		if ( d < 0 )
			d += inc1 ;
		else {
			y += ty ;
			d += inc2 ;
		}
		if ( iTag )
			oled_draw_point (y, x, NORMAL) ;
		else
			oled_draw_point (x, y, NORMAL) ;
		x += tx ;
	}
	return 0;
}

int32_t rt_device_oled_register(device_oled_t dev)
{
	assert(dev);
	dev->init 				= oled_init;
	dev->turn_on 			= oled_turn_on;
	dev->turn_off 			= oled_turn_off;
	dev->set_dir 			= oled_set_dir;
	dev->set_contrast 		= oled_set_contrast;
	dev->clear_screen 		= oled_clear_screen;
	dev->refresh_gram 		= oled_refresh_gram;
	dev->draw_point 		= oled_draw_point;
	dev->draw_line 			= oled_draw_line;
	dev->draw_bitmap 		= oled_draw_bitmap;
	dev->display_string 	= oled_display_string;
	dev->display_number 	= oled_display_number;
	
	return 0;
}



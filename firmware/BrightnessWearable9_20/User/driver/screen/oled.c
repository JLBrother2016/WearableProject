#include "oled.h"
#include "fonts.h"
#include "config.h"

/* global variable. */
struct device_oled device_oled;

/* -----------------------OLED12864�������Ŷ��� -------------------------------*/
	/* 
	 * OLED12864 �ӿ�������������
	 * 		DIN��MOSI�� = PA7			����ΪSPI
	 * 		CLK(SCK)    = PA5			����ΪSPI
	 *					= PB14 			����ΪSPI
	 *		CS			= PC4			����Ϊ����
	 *		D/C         = PB0			����Ϊ����
	 * 		RES 		= PC5			����Ϊ����
	 *
	 * STM32Ӳ��SPI�ӿ� = SPI1
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

/* ƬѡCS�õ�ѡ��  -----------------------------------------------------------*/
#define OLED_CS_LOW()       	OLED_CS_GPIO->BSRRH = OLED_CS_PIN
/* ƬѡCS�ø߲�ѡ�� ----------------------------------------------------------*/
#define OLED_CS_HIGH()      	OLED_CS_GPIO->BSRRL = OLED_CS_PIN
/* ��λRES�õ͸�λ -----------------------------------------------------------*/
#define OLED_RES_LOW()			OLED_RES_GPIO->BSRRH = OLED_RES_PIN
/* ��λRES�ø߲���λ ---------------------------------------------------------*/
#define OLED_RES_HIGH() 		OLED_RES_GPIO->BSRRL = OLED_RES_PIN
/* ����/����DC�õʹ������� ---------------------------------------------------*/
#define OLED_DC_LOW()			OLED_DC_GPIO->BSRRH = OLED_DC_PIN
/* ����/����DC�øߴ������� ---------------------------------------------------*/
#define OLED_DC_HIGH() 			OLED_DC_GPIO->BSRRL = OLED_DC_PIN

/* ��ʾ��Ⱥ͸߶ȶ��� --------------------------------------------------------*/
#define OLED_WIDTH    128
#define OLED_HEIGHT   64

/* ѡ���п�ʼ�ĵ�ַ */
#define __SET_COL_START_ADDR() do \
								{ \
									oled_write_byte(0x02, OLED_CMD); \
									oled_write_byte(0x10, OLED_CMD); \
								} while(0)
								
								
/**
  ******************************************************************************
  * OLED12864�Դ棬��Ÿ�ʽ���£�
  *  [Page0] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page1] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page2] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page3] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page4] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page5] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page6] Byte0 Byte1 ---------------------------------------Byte127
  *  [Page7] Byte0 Byte1 ---------------------------------------Byte127
  *
  *  OLED_GRAM[128][64]�е�128����������X���꣩��8����ҳ��ÿҳ����8�㣬��64�У�Y���꣩
  *  �Ӹߵ��Ͷ�Ӧ������С����
  *  һ��ͨ�õ㣨x, y����1���ʽΪ��
  * 		OLED_GRAM[x][7 - y/8] |= 1 << (7 - y%8)
  *  x��Χ0-127�� y��Χ0-63
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
	
	/* ʹ��GPIO ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	
	/* ���� SCK, MISO �� MOSI Ϊ���ù��� */
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
	
	/* ���ų�ʼ�� */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* ʹ��OLED���ƿڵ�ʱ�� */
	RCC_AHB1PeriphClockCmd(RCC_OLED_CTL_PORT, ENABLE);
	
	/* Ƭѡ��ѡ�� */
	OLED_CS_HIGH();
	
	/* CS RES DC����Ϊ������� */
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
	
	/* ��SPI1ʱ�� */
//	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
	
	/* ����SPI1Ӳ������ */
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* ���ݷ���2��ȫ˫�� */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;						/* SPI����ģʽ ������ģʽ */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;					/* ����λ���� �� 8λ */
	
	/* SPI_CPOL��SPI_CPHA���ʹ�þ���ʱ�Ӻ����ݲ��������λ��ϵ��
	   ��������: ���߿����Ǹߵ�ƽ,��2�����أ������ز�������)
	*/
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;							/* ʱ�������ز������� */
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;						/* ʱ�ӵĵ�2�����ز������� */
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;							/* Ƭѡ���Ʒ�ʽ��������� */

	/* ���ò�����Ԥ��Ƶϵ�� */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;					/* ����λ������򣺸�λ�ȴ� */
	SPI_InitStructure.SPI_CRCPolynomial = 7;							/* CRC����ʽ�Ĵ�������λ��Ϊ7�������̲��� */
	SPI_Init(SPI1, &SPI_InitStructure);

	SPI_Cmd(SPI1, DISABLE);												/* �Ƚ�ֹSPI  */
	SPI_Cmd(SPI1, ENABLE);												/* ʹ��SPI  */
	
	return 0;
}

int32_t oled_init(device_oled_t dev)
{ 
	OLED_CS_HIGH();
	OLED_DC_LOW();
	OLED_RES_HIGH();
	
	oled_write_byte(0xAE, OLED_CMD);		/* �ر�OLED�����ʾ�����ߣ� */
	oled_write_byte(0x00, OLED_CMD);		/* �����е�ַ��4bit */
	oled_write_byte(0x10, OLED_CMD);		/* �����е�ַ��4bit */
	oled_write_byte(0x40, OLED_CMD);		/* ������ʼ�е�ַ����5bit 0-63���� Ӳ����� */
	oled_write_byte(0x81, OLED_CMD);		/* ���öԱȶ����˫�ֽ��������1���ֽ��������2���ֽ��ǶԱȶȲ���0-255 */
	oled_write_byte(0xCF, OLED_CMD);		/* ���öԱȶȲ�����ȱʡCF */
	oled_write_byte(0xA1, OLED_CMD);		/* A0 ���е�ַ0ӳ�䵽SEG0; A1 ���е�ַ131ӳ�䵽SEG0 */
	oled_write_byte(0xC0, OLED_CMD);		/* C0 ������ɨ��,��COM0��COM63;  C8 : ����ɨ��, �� COM63�� COM0 */
	oled_write_byte(0xA6, OLED_CMD);		/* A6 : ����������ʾģʽ; A7 : ����Ϊ����ģʽ */
	oled_write_byte(0xA8, OLED_CMD);		/* ����COM·�� */
	oled_write_byte(0x3f, OLED_CMD);		/* 1 ->��63+1��· */
	oled_write_byte(0xD3, OLED_CMD);		/* ������ʾƫ�ƣ�˫�ֽ����*/
	oled_write_byte(0x00, OLED_CMD);		/* ��ƫ�� */
	oled_write_byte(0xd5, OLED_CMD);		/* ������ʾʱ�ӷ�Ƶϵ��/��Ƶ�� */
	oled_write_byte(0x80, OLED_CMD);		/* ���÷�Ƶϵ��,��4bit�Ƿ�Ƶϵ������4bit����Ƶ�� */
	oled_write_byte(0xD9, OLED_CMD);		/* ����Ԥ������� */
	oled_write_byte(0xF1, OLED_CMD);		/* [3:0],PHASE 1; [7:4],PHASE 2; */
	oled_write_byte(0xDA, OLED_CMD);		/* ����COM��Ӳ�����߷�ʽ */
	oled_write_byte(0x12, OLED_CMD);
	oled_write_byte(0xDB, OLED_CMD);		/* ���� vcomh ��ѹ���� */
	oled_write_byte(0x40, OLED_CMD);		/* [6:4] 000 = 0.65 x VCC; 0.77 x VCC (RESET); 0.83 x VCC  */
	oled_write_byte(0x20, OLED_CMD);
	oled_write_byte(0x02, OLED_CMD);
	oled_write_byte(0x8D, OLED_CMD);		/* ���ó��ã����¸�������ʹ�ã� */
	oled_write_byte(0x14, OLED_CMD);		/* 0x14 ʹ�ܳ��ã� 0x10 �ǹر� */
	oled_write_byte(0xA4, OLED_CMD);
	oled_write_byte(0xA6, OLED_CMD);
	dev->clear_screen();
	oled_write_byte(0xAF, OLED_CMD);		/* ��OLED��� */
	debug("OLED ��ʼ���ɹ���\r\n");
	return 0;
}

/**
  ******************************************************************************
  * @brief   ��SPI1����һ���ֽڣ�ͬʱ��MISO���߲����������ص�����
  * @param   _ch : ���͵��ֽ�ֵ
  * @retval  ��MISO���߲����������ص�����
  ******************************************************************************
  */
static uint8_t spi_send_byte(uint8_t _ch)
{
	/* �ȴ��ϸ�����δ������� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
	
	/* ͨ��SPIӲ������1���ֽ� */
	SPI_I2S_SendData(SPI1, _ch);
	
	/* �ȴ�����һ���ֽ�������� */
	while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
	
	/* ���ش�SPI���߶��������� */
	return SPI_I2S_ReceiveData(SPI1);
}

/**
  ******************************************************************************
  * @brief   ��oled����һ�����ݻ�����
  * @param   _chData  ���͵��ֽڣ� 
  *  		 _chCMD 0 Ϊ�������� �� 1 Ϊ��������
  * @retval  None
  ******************************************************************************
  */
static void oled_write_byte(uint8_t _chData, CMD_DAT_t _ch)
{
	assert_param(IS_CMD_DAT_VAL(_ch));
	
	/* ѡ��OLED */
	OLED_CS_LOW();
	if(_ch == OLED_DAT)
	{
		OLED_DC_HIGH(); 		/* �������� */
	}
	else if(_ch == OLED_CMD)
	{
		OLED_DC_LOW(); 			/* �������� */
	}
	
	/* SPI1 ����һ���ֽ� */
	spi_send_byte(_chData);
	
	OLED_CS_HIGH();			/* ��ѡ��OLED */
}


/**
  ******************************************************************************
  * @brief   ��OLED��ʾ
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_turn_on(void)
{
	oled_write_byte(0x8D, OLED_CMD);	/* ����DCDC������ó��ã�����һ��������ʹ�ã� */
	oled_write_byte(0x14, OLED_CMD);    /* ��DCDC�� 0x14��ʹ�ܳ��ã� 0x10�ǹر� */
	oled_write_byte(0xAF, OLED_CMD);    /* ��OLED ��� */
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   �ر�OLED��ʾ
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_turn_off(void)
{
	oled_write_byte(0x8D, OLED_CMD);	/* ����DCDC������ó��ã�����һ��������ʹ�ã� */
	oled_write_byte(0x10, OLED_CMD); 	/* ��DCDC�� 0x14��ʹ�ܳ��ã� 0x10�ǹر� */
	oled_write_byte(0xAF, OLED_CMD);	/* �ر�OLED ��� */
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   ������ʾ����
  * @param   _ucDir = 0��ʾ�������� 1��ʾ��ת180��
  * @retval  None
  ******************************************************************************
  */
int32_t oled_set_dir(uint8_t _ucDir)
{
	if(_ucDir == 0)
	{
		oled_write_byte(0xA0, OLED_CMD);	/* A0 ���е�ַ0ӳ�䵽SEG0; A1 ���е�ַ127ӳ�䵽SEG0 */
		oled_write_byte(0xC0, OLED_CMD);    /* C0 ������ɨ��,��COM0��COM63;  C8 : ����ɨ��, �� COM63�� COM0 */
	}
	else
	{
		oled_write_byte(0xA1, OLED_CMD);	/* A0 ���е�ַ0ӳ�䵽SEG0; A1 ���е�ַ127ӳ�䵽SEG0 */
		oled_write_byte(0xC8, OLED_CMD);	/* C0 ������ɨ��,��COM0��COM63;  C8 : ����ɨ��, �� COM63�� COM0 */
	}
	
	return 0;
}


/**
  ******************************************************************************
  * @brief   ������ʾ�Աȶ�
  * @param   _ucValue �ԱȶȲ���0-255
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
  * @brief   Refresh the graphic ram ��OLED_GRAM[128][8]ˢ�� OLED����
  *          �ú�����SysTick�ж�����ÿ10ms������һ�Σ�Ҳ����ˢ��Ƶ��Ϊ100HZ
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_refresh_gram(void)
{
	uint8_t i, j;
	
	for(i = 0; i < 8; i++) 				/* ����Page */
	{
		oled_write_byte(0xB0 + i, OLED_CMD);
		
		__SET_COL_START_ADDR();			/* ѡ���е�ַ��SH1106��COL2��ʼ */
		
		for(j = 0; j < 128; j++)		/* ������ */
		{
			oled_write_byte(OLED_GRAM[j][i], OLED_DAT);
		}
	}
	
	return 0;
}

/**
  ******************************************************************************
  * @brief   clear the screen ��������������Ļ�Ǻ�ɫ�ģ���û����һ��
  * 		 ����ʱOLED�ǹ�����
  * @param   None
  * @retval  None
  ******************************************************************************
  */
int32_t oled_clear_screen(void)
{
	uint8_t i, j;
	
	for(i = 0; i < 8; i++)								/* ����Page */
	{
		oled_write_byte(0xB0 + i, OLED_CMD);
		
		__SET_COL_START_ADDR();
		
		for(j = 0; j < 128; j++)						/* ������ */	
		{
			OLED_GRAM[j][i] = 0x00;
		}
	}
	
//	oled_refresh_gram();							/* ˢ�� */
	return 0;
}


/**
  ******************************************************************************
  * @brief   draw a point on the screen �����OLED��ʾ�ĺ��ĺ���
  * @param   _chXpos specify the X position
  * 		 _chYpos specify the Y position
  *  		 _chPoint 1 ��� 0 ���
  * @retval  None
  ******************************************************************************
  */
int32_t oled_draw_point(uint8_t _chXpos, uint8_t _chYpos, DISPLAY_MODE_t _chPoint)
{
	uint8_t chPos, chBx, chTemp;
	
	assert_param(IS_DISPLAY_MODE_VAL(_chPoint));
	
	/* �������ĺ�����128*64 */
	if(_chXpos > 127 || _chYpos > 63)
	{
		return -1;
	}
	
	chPos = 7 - _chYpos / 8;	/* ����ҳ��ַ�е��е�ַ */
	chBx = _chYpos % 8;        	/* ����ҳ��ַ */
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
  * @param  chXpos: Specifies the X position�� ����12864��Ļ����Χ��0-127��
  *         chYpos: Specifies the Y position�� ����12864ƽ������Χ��0-63��
  *         chSize: ѡ������
  *         chMode 0 ������ʾ�� 1 ������ʾ
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
		if (_chSize == SIZE_12) 				/* ʹ��1206���� */
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
		
		else if(_chSize == SIZE_16)   		/* ʹ��1608���� */
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
  *         _chNum: ��ֵ����
  *         _chLen: ���ֵ�λ��
  *  		_chSize: �����С
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
  *         chSize �����С
  * 		chMode ��ʾģʽ 0 ���ԣ� 1 ����
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
  * @brief  draw a line on the screen   ����Bresenham�㷨����2��仭һ��ֱ��     
  * @param  _usX1, _usY1 ����ʼ������
  *   		_usX2, _usY2 ����ֹ������
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
	
	/* ���� Bresenham �㷨����2��仭һ��ֱ�� */
	oled_draw_point(_usX1, _usY1, NORMAL);
	
	/* ��������غϣ���������Ķ�����*/
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
	
	if ( dx < dy ) {  /*���dyΪ�Ƴ������򽻻��ݺ����ꡣ*/
		uint16_t temp;
		iTag = 1 ;
		temp = _usX1; _usX1 = _usY1; _usY1 = temp;
		temp = _usX2; _usX2 = _usY2; _usY2 = temp;
		temp = dx; dx = dy; dy = temp;
	}
	
	tx = _usX2 > _usX1 ? 1 : -1 ;    /* ȷ������1���Ǽ�1 */
	ty = _usY2 > _usY1 ? 1 : -1 ;
	x = _usX1 ;
	y = _usY1 ;
	inc1 = 2 * dy ;
	inc2 = 2 * ( dy - dx );
	d = inc1 - dx ;
	
	while ( x != _usX2 ) {    /* ѭ������ */
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



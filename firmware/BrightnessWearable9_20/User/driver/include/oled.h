#ifndef _OLED_H_
#define _OLED_H

#include <stm32f4xx.h>

#ifdef __cplusplus
	extern "C" {
#endif

		/* 显示模式枚举 --------------------------------------------------------------*/
typedef enum
{
	INVERSE = 0, 
	NORMAL  = 1
} DISPLAY_MODE_t;

#define IS_DISPLAY_MODE_VAL(VAL) ((VAL) == (INVERSE) || \
								  (VAL) == (NORMAL))



/* 命令数据枚举 --------------------------------------------------------------*/
typedef enum
{
	OLED_CMD = 0,
	OLED_DAT = 1
} CMD_DAT_t;

#define IS_CMD_DAT_VAL(VAL) ((VAL) == (OLED_CMD) || \
							 (VAL) == (OLED_DAT))

/* 字符大小枚举 --------------------------------------------------------------*/
typedef enum
{
	SIZE_12 = 12,
	SIZE_16 = 16
} CHAR_SIZE_t;

#define IS_CHAR_SIZE_VAL ((VAL) == (SIZE_12) || \
						  (VAL) == (SIZE_16))

typedef struct device_oled* device_oled_t;

struct device_oled {
	int32_t (*init)(device_oled_t dev);
	int32_t (*turn_on)(void);
	int32_t (*turn_off)(void);
	int32_t (*set_dir)(uint8_t _ucDir);
	int32_t (*set_contrast)(uint8_t _ucValue);
	int32_t (*clear_screen)(void);
	int32_t (*refresh_gram)(void);
	int32_t (*draw_point)(uint8_t _chXpos, uint8_t _chYpos, DISPLAY_MODE_t _chPoint);
	int32_t (*draw_line)(uint16_t _usX1, uint16_t _usY1, uint16_t _usX2, uint16_t _usY2);
	int32_t (*draw_bitmap)(uint8_t _chXpos, uint8_t _chYpos, const uint8_t *_pchBmp, uint8_t _chWidth, uint8_t _chHeight);
	int32_t (*display_string)(uint8_t _chXpos, uint8_t _chYpos, const char *_pchString, CHAR_SIZE_t _chSize, DISPLAY_MODE_t _chMode);
	int32_t (*display_number)(uint8_t _chXpos, uint8_t _chYpos, uint32_t _chNum, uint8_t _chLen, CHAR_SIZE_t _chSize);
};


extern struct device_oled device_oled;
extern int32_t stm32_hw_oled_spi_init(void);
extern int32_t rt_device_oled_register(device_oled_t dev);

		
#ifdef __cplusplus
}
#endif

#endif

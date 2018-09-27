#ifndef _RTC_H_
#define _RTC_H_

#include <stm32f4xx.h>
#include <rtthread.h>

#ifdef __cplusplus
	extern "C" {
#endif

typedef struct rtc {
	uint32_t  				year;
	uint8_t 				month;
	uint8_t 				day;
	uint8_t 				weekday;
	uint8_t 				hour;
	uint8_t 				min; 
	uint8_t 				sec;
} rtc_t;


extern int32_t RTC_set_time(uint8_t hour, uint8_t min, uint8_t sec, uint8_t ampm);
extern int32_t RTC_set_data(uint8_t year, uint8_t month, uint8_t day, uint8_t weekday);
extern int32_t RTC_set_wakeup_1s(void);
extern int32_t RTC_init(void);
extern int32_t get_rtc_info(rtc_t *info);

#ifdef __cplusplus
}
#endif

#endif

#ifndef _UART2_H_
#define _UART2_H_

#include <stm32f4xx.h>

struct uart2_buffer {
	char buffer[1024];
	uint32_t pos;
};

#endif

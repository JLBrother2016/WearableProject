#ifndef _CPUUSAGE_H_
#define _CPUUSAGE_H_

#include <rtthread.h>

void cpu_usage_init(void);
void cpu_usage_get(rt_uint8_t *major, rt_uint8_t *minor);


#endif

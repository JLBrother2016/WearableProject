#ifndef _WINDOW_H
#define _WINDOW_H
#include "arm_math.h"
#include "arm_const_structs.h"
#define TWOPI  6.28318530717959

void hanning( float *win, int N);
void hamming( float *win, int N);
void apply_window(float* data, float* window, int window_len);  

#endif

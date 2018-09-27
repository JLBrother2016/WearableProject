#ifndef _WINDOW_H
#define _WINDOW_H
#include "arm_math.h"
#include "arm_const_structs.h"

#define TWOPI  6.28318530717959
#define FRAME_LEN 256
#define FRAME_MOV 80
#define BUFFER_LEN 25600
#define FRAME_NUM ((BUFFER_LEN-FRAME_LEN)/(FRAME_LEN-FRAME_MOV) + 1)

void hanning( float *win, int N);
void hamming( float *win, int N);
void apply_window(float* data, float* window, int window_len);  

#endif
#include "window.h"


void hanning( float *win, int N)  
{     
    int half = 0;  
    if ( N % 2 == 0 )  
    {  
        half = N / 2;  
  
        for (int i = 1; i <= half; ++i)  
        {  
            win[i - 1] = 0.5 - 0.5*arm_cos_f32(TWOPI*i / (N + 1.0));  
        }  
  
        int index = half + 1;  
        for (int i = half; i >= 1; i--)  
        {  
            win[index - 1] = win[i - 1];  
            index++;  
        }  
  
    }  
    else  
    {  
        half = (N + 1) / 2;  
  
        for (int i = 1; i <= half; ++i)  
        {  
            win[i - 1] = 0.5 - 0.5*arm_cos_f32(TWOPI*i / (N + 1.0));  
        }  
  
        int index = half + 1;  
        for (int i = half-1; i >= 1; i--)  
        {  
            win[index - 1] = win[i - 1];  
            index++;  
        }  
  
    }  
}  
void hamming( float *win, int N)  
{  
    int half = 0;  
    if ( N % 2 == 0 )  
    {  
        half = N / 2;  
  
        for (int i = 1; i <= half; ++i)  
        {  
            win[i - 1] = 0.54 - 0.46*arm_cos_f32(TWOPI*i / (N + 1.0));  
        }  
  
        int index = half + 1;  
        for (int i = half; i >= 1; i--)  
        {  
            win[index - 1] = win[i - 1];  
            index++;  
        }  
  
    }  
    else  
    {  
        half = (N + 1) / 2;  
  
        for (int i = 1; i <= half; ++i)  
        {  
            win[i - 1] = 0.54 - 0.46*arm_cos_f32(TWOPI*i / (N + 1.0));  
        }  
  
        int index = half + 1;  
        for (int i = half-1; i >= 1; i--)  
        {  
            win[index - 1] = win[i - 1];  
            index++;  
        }  
  
    }  
}  
void apply_window(float* data, float* window, int window_len)  
{  
    for(int i = 0; i< window_len; i++)  
    {  
        data[i] = data[i] * window[i];  
    }  
  
}
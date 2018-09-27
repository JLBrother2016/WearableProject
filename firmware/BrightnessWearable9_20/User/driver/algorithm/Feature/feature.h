#ifndef _FEATURE_H
#define _FEATURE_H

#include "config.h"

#define WINDOW_LEN 256
#define FFT_LEN 128


float get_energy_window(float *in_data);
float get_entropy_brightness_window(float *in_data, int32_t *brightness);
void get_formant_window(float *in_data, float *in_formant, uint32_t *out_formant);

#endif

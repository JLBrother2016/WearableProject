#include "feature.h"
#include "window.h"
#include "arm_const_structs.h"
#include "arm_math.h"

float in_data_fft[WINDOW_LEN*2];
float out_data_fft[WINDOW_LEN];
uint32_t ifftFlag = 0; 								//控制是否FFT变换或者FFT逆变换，0为正变换
uint32_t doBitReverse = 1; 							//旋转因数和位反转标志位，1为翻转

void get_hanning_window(float *in_data){
	float hanning_window[WINDOW_LEN];

	hanning(hanning_window, WINDOW_LEN);
	for (int i = 0; i < WINDOW_LEN; ++i){
		in_data[i] = hanning_window[i] * in_data[i];
	}
}
void get_hamming_window(float *in_data){
	float hamming_window[WINDOW_LEN];

	hamming(hamming_window, WINDOW_LEN);
	for (int i = 0; i < WINDOW_LEN; ++i){
		in_data[i] = hamming_window[i] * in_data[i];
	}
}

void pre_filter_window(float *in_data, float *out_data)
{
	out_data[0] = in_data[0];
	for (int i = 1; i < WINDOW_LEN; ++i){
		out_data[i] = in_data[i] - 0.99*in_data[i-1];
	}	
}

float get_energy_window(float *in_data){
	float resualt_energy = 0.0;

	get_hanning_window(in_data);
	for (int i = 0; i < WINDOW_LEN; ++i){
		resualt_energy += (in_data[i]*10) * (in_data[i]*10);
	}
	return resualt_energy;
}

float get_entropy_brightness_window(float *in_data, int32_t *brightness){
	float resualt_entropy = 0.0;
	float sp_entropy = 0.0;
	float sp_brightness = 0.0;

	for (int i = 0; i < WINDOW_LEN; ++i){
//		debug("%d\n", (int)((in_data[i]*1000)*(in_data[i]*1000)));
		in_data_fft[i*2] = in_data[i];
		in_data_fft[i*2+1] = 0;
	}
	/* CFFT变换 */ 
	arm_cfft_f32(&arm_cfft_sR_f32_len256, in_data_fft, ifftFlag, doBitReverse);
	/* 求解模值  */ 
	arm_cmplx_mag_f32(in_data_fft, out_data_fft, WINDOW_LEN);
	
	for (int i = 0; i <= FFT_LEN; ++i){
//		debug("%d\n", (int)(out_data_fft[i]*10000));
		out_data_fft[i] = out_data_fft[i] * out_data_fft[i]; 
		sp_brightness += 3125*i*out_data_fft[i];    //放大了100倍，31.25是采样率8K时，256个点FFT后对应的频率刻度大小
		sp_entropy += out_data_fft[i];
	}
	for (int i = 0; i <= FFT_LEN; ++i){
		in_data_fft[i] = out_data_fft[i]/sp_entropy;
		resualt_entropy -= in_data_fft[i] * log(in_data_fft[i]);
	}
	
	*brightness = (int) (sp_brightness / sp_entropy);
	
//	debug("%d\n", brightness);

	return resualt_entropy;
}

void get_formant_window(float *in_data, float *in_formant, uint32_t *out_formant){

	pre_filter_window(in_data, in_formant);
	get_hamming_window(in_formant);
	for (int i = 0; i < WINDOW_LEN; ++i){
		in_data_fft[i*2] = in_formant[i];
		in_data_fft[i*2+1] = 0;
	}
	/* CFFT变换 */ 
	arm_cfft_f32(&arm_cfft_sR_f32_len256, in_data_fft, ifftFlag, doBitReverse);
	/* 求解模值  */ 
	arm_cmplx_mag_f32(in_data_fft, out_data_fft, WINDOW_LEN);
	for (int i = 0; i < FFT_LEN; ++i){
		out_data_fft[i] = log(out_data_fft[i]);
		in_data_fft[i*2] = out_data_fft[i];
		in_data_fft[i*2+1] = 0;
		in_data_fft[i*2+128] = 0;
	}
	ifftFlag = 1;  												//FFT逆变换
	arm_cfft_f32(&arm_cfft_sR_f32_len128, in_data_fft, ifftFlag, doBitReverse);
	for(int i = 6; i < 123; ++i){
		in_data_fft[i*2+1] = 0;
		in_data_fft[i*2] = 0;
	}
	ifftFlag = 0;  												//FFT变换
	arm_cfft_f32(&arm_cfft_sR_f32_len128, in_data_fft, ifftFlag, doBitReverse);
	int j = 0;
	for(int i = 0; i < 5; ++i){
		out_formant[i] = 0;
	}
	for (int i = 0; i < FFT_LEN; ++i){
		if((i != 0) && (i != (FFT_LEN -1))){
			if(in_data_fft[i*2] > in_data_fft[(i-1)*2] &&
			 in_data_fft[i*2] > in_data_fft[(i+1)*2]){
//				debug("%d\n", i*3125);
				 out_formant[j] = i*3125;    //3125把频率刻度31.25放大了100倍，31.25是采样率8K时，256个点FFT后对应的频率刻度大小 
//				debug("%d\n", out_formant[j]);
				j++;
			}
		}
	}
}


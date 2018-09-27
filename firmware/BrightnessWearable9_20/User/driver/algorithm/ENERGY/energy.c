#include "energy.h"
#include "window.h"
#include "config.h"
#include "arm_const_structs.h"
#include "arm_math.h"

float buffer_voice[2560];
float fft_indata[512];
float fft_outdata[256];
float fft_computer[130];
float p_entropy[130];
float result_entropy[10];

void get_energy(float *in_data, float *out_engergy)
{
	float temp;
	for(int i = 0; i < 10; i++){
		debug("%d\n", (int)(out_engergy[i]*10000));
	}
	
	for(int i = 0; i < 10; i++){
		out_engergy[i] = 0;
		debug("it is:%dwindow\n", i);
		for(int j = 0; j < 256; j++){
			in_data[j] = buffer_voice[i*256 + j];
			debug("%d\n", (int)((in_data[j]*1000)*(in_data[j]*1000)));
			
			fft_indata[j*2] = in_data[j];
			fft_indata[j*2+1] = 0;
		}
		/* CFFT变换 */ 
		arm_cfft_f32(&arm_cfft_sR_f32_len256, fft_indata, 0, 1);
		/* 求解模值  */ 
		arm_cmplx_mag_f32(fft_indata, fft_outdata, 256);
		
		for(int j = 0; j < 129; j++){
			fft_computer[j] = fft_outdata[j]*fft_outdata[j];
			temp = temp + fft_computer[j]; 
		}
		for(int j = 0; j < 129; j++){
			p_entropy[j] = fft_computer[j]/temp;
			result_entropy[i] = result_entropy[i] - (p_entropy[j]*log(p_entropy[j]));
		}
		for(int j = 0; j < 256; j++){
			out_engergy[i] = out_engergy[i] + (in_data[j]*10.0)*(in_data[j]*10.0);
			debug("%d\n", (int)(fft_outdata[j]*10000));
		}
	}
	
	debug("result_entropy:\n");
	for(int i = 0; i < 10; i++){
//		debug("%d\n", (int)(out_engergy[i]*100));
		debug("%d\n", (int)(result_entropy[i]*10000));
	}
}

//void get_entropy()
//{

//}
void get_Preemphasis(float *indata, int16_t length)
{
	float temp;
	for(int i = 1; i < length; ++i){
		temp = indata[i-1];
		indata[i] = indata[i] - 0.99*temp;
	}
}

void get_formant(float *in_data, float *out_formant)
{
	float temp;
	for(int i = 0; i < 1; i++){
		debug("it is:%dwindow\n", i);
		for(int j = 0; j < 256; ++j){
			in_data[j] = buffer_voice[i*256 + j];
			if( j != 0){
				temp = in_data[i-1];
				in_data[i] = in_data[i] - 0.99*temp;
			}
			fft_indata[j*2] = in_data[j];
			fft_indata[j*2+1] = 0;				
		}
		/* CFFT变换 */ 
		arm_cfft_f32(&arm_cfft_sR_f32_len256, fft_indata, 0, 1);
		/* 求解模值  */ 
		arm_cmplx_mag_f32(fft_indata, fft_outdata, 256);
		for(int j = 0; j < 256; j++){
			fft_outdata[j] = log(fft_outdata[j]);
			debug("%d\n", (int)(fft_outdata[j]*10000));
		}	
		arm_cfft_f32(&arm_cfft_sR_f32_len256, fft_indata, 1, 1);
		for(int j = 0; j < 256; j++){
			if(j > 5 && j < 251)
				fft_indata[j*2] = 0;
		}
		arm_cfft_f32(&arm_cfft_sR_f32_len256, fft_indata, 0, 1);
		for(int j = 0; j < 256; j++){
			if(j != 0 && j != 255){
				if(fft_indata[j*2] > fft_indata[(j-1)*2] && fft_indata[j*2] > fft_indata[(j+1)*2])
					debug("%d\n",j);
			}
//			debug("%d\n", (int)(fft_indata[j*2]*10000));
		}
	}
}

/* 
	二阶低通滤波器。

*/

#pragma once

typedef struct {
	float cutoff_freq;
	
	float a1;
	float a2;

	float b0;
	float b1;
	float b2;
	
	float delay_element_1;
	float delay_element_2;
		
} LowPassFilter2p_t;

typedef struct {
	float notch_freq;
	float bandwidth;
	
	float a1;
	float a2;

	float b0;
	float b1;
	float b2;
	float delay_element_1;
	float delay_element_2;
		
} NotchFilter_t;

void LowPassFilter2p_Init(LowPassFilter2p_t *f, float sample_freq, float cutoff_freq);
float LowPassFilter2p_Apply(LowPassFilter2p_t *f, float sample);
float LowPassFilter2p_Reset(LowPassFilter2p_t *f, float sample);

void NotchFilter_Init(NotchFilter_t *f, float sample_freq, float notch_freq, float bandwidth);
float NotchFilter_Apply(NotchFilter_t *f, float sample);
float NotchFilter_Reset(NotchFilter_t *f, float sample);
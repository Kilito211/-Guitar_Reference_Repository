#ifndef _ADCX_H_
#define _ADCX_H_
#include "stm32f10x.h" // Device header

// ADC б└ид??????
// ?????? ADC1/2/3
#define ADCx ADC1
#define ADC_CLK RCC_APB2Periph_ADC1

void adcx_init(void);
u16 adc_get_value(uint8_t adc_channel, uint8_t adc_sample_time);

#endif

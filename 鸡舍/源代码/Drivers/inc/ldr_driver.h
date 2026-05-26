#ifndef LDR_DRIVER_H
#define LDR_DRIVER_H

#include "stm32f10x.h"
#include "adcx_driver.h"
#include "timer_driver.h"
#include "math.h"

#define LDR_READ_TIMES 10 // 光照传感器ADC循环读取次数

// LDR GPIO宏定义
#define LDR_GPIO_CLK RCC_APB2Periph_GPIOA
#define LDR_GPIO_PORT GPIOA
#define LDR_GPIO_PIN GPIO_Pin_4

// ADC 通道宏定义
#define LDR_ADC_CHANNEL ADC_Channel_4

// #define    ADC_IRQ                       ADC3_IRQn
// #define    ADC_IRQHandler                ADC3_IRQHandler

void ldr_init(void);
uint16_t ldr_average_date(void);
uint16_t ldr_lux_data(void);

#endif // LDR_DRIVER_H

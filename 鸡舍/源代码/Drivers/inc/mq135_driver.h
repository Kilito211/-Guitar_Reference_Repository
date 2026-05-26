#ifndef MQ135_DRIVER_H
#define MQ135_DRIVER_H

#include "workqueue.h"
#include "stm32f10x_adc.h"

#define MQ135_READ_TIMES 10 // MQ-135传感器ADC循环读取次数
#define MQ135_MODE 1        // 1:模拟AO  0:数字DO

#if MQ135_MODE
#define MQ135_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOA
#define MQ135_SWITCH_PORT GPIOA
#define MQ135_SWITCH_GPIO_PIN (GPIO_Pin_5)
#define MQ135_ADC_CHANNEL ADC_Channel_5
#define MQ135_GPIO_MODE GPIO_Mode_AIN
#else // 暂未定义
#define MQ135_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOA
#define MQ135_SWITCH_PORT GPIOA
#define MQ135_SWITCH_GPIO_PIN (GPIO_Pin_5)
#define MQ135_GPIO_MODE GPIO_Mode_IPU
#endif // MODE

void mq135_init(void);
uint16_t mq135_get_data(void);
float mq135_get_data_ppm(void);

#endif // MQ135_DRIVER_H

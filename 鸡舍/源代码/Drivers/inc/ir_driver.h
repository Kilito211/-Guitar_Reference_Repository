#ifndef IR_DRIVER_H
#define IR_DRIVER_H

#include "workqueue.h"

#define IR_READ_TIMES 10 // 火焰传感器ADC循环读取次数
#define IR_MODE 1        // 1:模拟信号 0数字信号

#if IR_MODE
#define IR_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOA
#define IR_SWITCH_PORT GPIOA
#define IR_SWITCH_GPIO_PIN (GPIO_Pin_6)
#define IR_ADC_CHANNEL ADC_Channel_6
#define IR_GPIO_MODE GPIO_Mode_AIN
#else // 暂未定义
#define IR_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOA
#define IR_SWITCH_PORT GPIOA
#define IR_SWITCH_GPIO_PIN (GPIO_Pin_6)
#define IR_GPIO_MODE GPIO_Mode_IPU
#endif // MODE

void ir_init(void);
uint16_t ir_fire_data();

#endif // IR_DRIVER_H

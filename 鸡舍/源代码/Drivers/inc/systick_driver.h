#ifndef SYSTICK_DRIVER_H
#define SYSTICK_DRIVER_H

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"

void systick_delay_us(uint32_t us);
void systick_delay_ms(uint32_t ms);

#endif // SYSTICK_DRIVER_H

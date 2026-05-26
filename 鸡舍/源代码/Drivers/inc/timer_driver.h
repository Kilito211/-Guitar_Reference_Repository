#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include "stm32f10x.h"

void timer_init(uint32_t prescaler);
void timer_delay_us(uint32_t us);
void timer_delay_ms(uint32_t ms);
void tim3_pwm_init(uint32_t arr, uint16_t prescaler);
void set_light_brightness(uint32_t percent);
void timer_debounce_init(uint32_t ms);

#endif // TIMER_DRIVER_H

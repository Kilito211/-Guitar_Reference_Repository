#ifndef LED_DRIVER_H
#define LED_DRIVER_H

#include "typedef.h"

#define LED_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOC
#define LED_SWITCH_PORT GPIOC
#define LED_SWITCH_GPIO_PIN (GPIO_Pin_13)

#define LED_SWITCH_ON (PCout(13) = 0)
#define LED_SWITCH_OFF (PCout(13) = 1)
#define LED_SWUTCH_STAT (PCout(13) == 1)

void led_init(void);
void led_reverse(void);
void led_on(void);
void led_off(void);

#endif // LED_DRIVER_H

#ifndef BEEP_DRIVER_H
#define BEEP_DRIVER_H

#include "typedef.h"

#define BEEP_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOA
#define BEEP_SWITCH_PORT GPIOA
#define BEEP_SWITCH_GPIO_PIN (GPIO_Pin_7)

#define BEEP_SWITCH_ON (PAout(7) = 0)
#define BEEP_SWITCH_OFF (PAout(7) = 1)
#define BEEP_SWUTCH_STAT (PAin(7) == 1)

void beep_init(void);
void beep_on(void);
void beep_off(void);
void beep_reverse(void);

#endif // BEEP_DRIVER_H

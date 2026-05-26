#ifndef RELAY_DRIVER_H
#define RELAY_DRIVER_H

#include "typedef.h"

#define RELAY_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOB
#define RELAY_SWITCH_PORT GPIOB
#define RELAY_SWITCH_GPIO_PIN (GPIO_Pin_1)

#define RELAY_SWITCH_ON (PBout(6) = 1)
#define RELAY_SWITCH_OFF (PBout(6) = 0)
#define RELAY_SWUTCH_STAT (PBin(6) == 1)

void relay_gpio_init(void);
void relay_on(void);
void relay_off(void);

#endif // RELAY_DRIVER_H


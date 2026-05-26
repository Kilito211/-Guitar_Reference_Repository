#ifndef DHT11_DRIVER_H
#define DHT11_DRIVER_H

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "timer_driver.h"

#define DHT11_SWITCH_SYSCTL_PERIPH_CLK RCC_APB2Periph_GPIOA
#define DHT11_SWITCH_PORT GPIOA
#define DHT11_SWITCH_GPIO_PIN (GPIO_Pin_8)

uint8_t dht11_init(void);
uint8_t dht11_read_data(uint8_t *data);

#endif // DHT11_DRIVER_H
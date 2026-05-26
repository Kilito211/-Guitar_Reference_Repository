/**
 * @file relay_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief  继电器控制驱动
 * @version 0.1
 * @date 2026-01-27
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "relay_driver.h"

/**
 * @brief 初始化继电器
 *
 */
void relay_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RELAY_SWITCH_SYSCTL_PERIPH_CLK, ENABLE); // 使能PB端口时钟

    GPIO_InitStructure.GPIO_Pin = RELAY_SWITCH_GPIO_PIN; // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;     // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;    // IO口速度为50MHz
    GPIO_Init(RELAY_SWITCH_PORT, &GPIO_InitStructure);   // 根据设定参数初始化GPIO B1

    RELAY_SWITCH_OFF;
}

/**
 * @brief 继电器打开
 */
void relay_on(void)
{
    RELAY_SWITCH_ON;
}

void relay_off(void)
{
    RELAY_SWITCH_OFF;
}

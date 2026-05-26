/**
 * @file led_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief  LED灯驱动
 * @version 0.1
 * @date 2026-01-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "led_driver.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/**
 * @brief LED初始化
 */
void led_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LED_SWITCH_SYSCTL_PERIPH_CLK, ENABLE); // 使能PB端口时钟

    GPIO_InitStructure.GPIO_Pin = LED_SWITCH_GPIO_PIN; // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;  // IO口速度为50MHz
    GPIO_Init(LED_SWITCH_PORT, &GPIO_InitStructure);   // 根据设定参数初始化GPIO B1

    LED_SWITCH_OFF;
}

void led_on(void)
{
	PCout(13) &= 0;
}

void led_off(void)
{
	PCout(13) |= 1;
}

/**
 * @brief 翻转LED灯
 */
void led_reverse(void)
{
    PCout(13) ^= 0x01;
}

/**
 * @file beep_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief  蜂鸣器驱动函数
 * @version 0.1
 * @date 2026-01-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "beep_driver.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

/**
 * @brief 初始化蜂鸣器
 */
void beep_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(BEEP_SWITCH_SYSCTL_PERIPH_CLK, ENABLE); // 使能PB端口时钟

    GPIO_InitStructure.GPIO_Pin = BEEP_SWITCH_GPIO_PIN; // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 开漏输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;   // IO口速度为50MHz
    GPIO_Init(BEEP_SWITCH_PORT, &GPIO_InitStructure);   // 根据设定参数初始化GPIO B1

    BEEP_SWITCH_OFF;
}

void beep_on(void)
{
    BEEP_SWITCH_ON;
}

void beep_off(void)
{
    BEEP_SWITCH_OFF;
}

/**
 * @brief 翻转蜂鸣器
 */
void beep_reverse(void)
{
    PAout(7) ^= 0x01;
}

/**
 * @file systick_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief 系统滴答计时器驱动
 * @version 0.1
 * @date 2025-12-05
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "systick_driver.h"

void systick_delay_us(uint32_t us)
{
    ///< 清楚标记
    SysTick->CTRL = 0;
    // 清除 VAL 寄存器
    SysTick->VAL = 0;
    // 配置 LOAD 寄存器
    SysTick->LOAD = us * 9;
    // 启动定时器
    SysTick->CTRL |= 0x01;
    // 等待CTRL的第十六位变为1(倒计时结束)
    while (!(SysTick->CTRL & (0x01 << 16)))
        ;
    // 停止寄存器
    SysTick->CTRL &= ~0x01;
}

void systick_delay_ms(uint32_t ms)
{
    while (ms--)
        systick_delay_us(1000);
}

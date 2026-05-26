/**
 * @file timer_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief  timer定时器驱动
 * @version 0.1
 * @date 2025-12-07
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "stm32f10x.h"
#include "timer_driver.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "misc.h"

#define TIMER TIM2
#define DELAY_TIMER TIM4

/**
 * @brief 定时器初始化
 *
 * @param prescaler 预分频
 */
void timer_init(uint32_t prescaler)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    TIM_TimeBaseInitStruct.TIM_Prescaler = prescaler - 1; // 72-1 → 1MHz
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF;           // 给一个有效 ARR
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;

    TIM_TimeBaseInit(DELAY_TIMER, &TIM_TimeBaseInitStruct);

    // 清楚更新标志
    TIM_ClearFlag(DELAY_TIMER, TIM_FLAG_Update);

    // 停止定时器
    TIM_Cmd(DELAY_TIMER, DISABLE);
}

/**
 * @brief 微秒定时器
 *
 * @param us 微秒
 */
void timer_delay_us(uint32_t us)
{
    DELAY_TIMER->ARR = us * 72 / (DELAY_TIMER->PSC + 1);
    DELAY_TIMER->CNT = 0;
    TIM_Cmd(DELAY_TIMER, ENABLE); // 启动定时器
    while (TIM_GetFlagStatus(DELAY_TIMER, TIM_FLAG_Update) == RESET)
        ;

    TIM_Cmd(DELAY_TIMER, DISABLE); // 停止定时器
    TIM_ClearFlag(DELAY_TIMER, TIM_FLAG_Update);
}

/**
 * @brief 毫秒定时器
 *
 * @param ms 毫秒
 */
void timer_delay_ms(uint32_t ms)
{
    while (ms--)
        timer_delay_us(1000);
}

/**
 * @brief 初始化消抖定时器
 *
 * @param ms 消耗时间
 */
void timer_debounce_init(uint32_t ms)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 定时器基础配置（72MHz时钟，预分频7200→10kHz计数）
    TIM_TimeBaseInitStruct.TIM_Prescaler = 7200 - 1; // 72MHz/7200 = 10kHz
    TIM_TimeBaseInitStruct.TIM_Period = ms * 10 - 1; // 10kHz * 10ms = 100次计数
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIMER, &TIM_TimeBaseInitStruct);

    // 清除更新标志
    TIM_ClearFlag(TIMER, TIM_FLAG_Update);
    // 使能定时器更新中断
    TIM_ITConfig(TIMER, TIM_IT_Update, ENABLE);

    // 配置NVIC（TIM2中断）
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 优先级低于外部中断
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 初始停止定时器
    TIM_Cmd(TIMER, DISABLE);
}

/**
 * @brief 将TIM3设置为PWM，用于调节灯光亮度
 *
 * @param arr 重装载值
 * @param prescaler 预分频器
 */
void tim3_pwm_init(uint32_t arr, uint16_t prescaler)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    // PA6复用推挽输出(TIM3_CH1)
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // 基础定时器配置
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period = arr;
    TIM_TimeBaseStructure.TIM_Prescaler = prescaler - 1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    // PWM配置模式
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputNState_Enable;
    TIM_OCInitStructure.TIM_Pulse = 0;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCNPolarity_High;
    TIM_OC1Init(TIM3, &TIM_OCInitStructure);

    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief 设置占空比
 *
 * @param percent 占空比
 */
void set_light_brightness(uint32_t percent)
{
    if (percent > 100)
        percent = 100;
    TIM_SetCompare1(TIM3, percent * 10); // 设置占空比
}

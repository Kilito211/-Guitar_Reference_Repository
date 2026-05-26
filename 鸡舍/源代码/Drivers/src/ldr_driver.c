/**
 * @file ldr_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief  光敏电阻传感器驱动
 * @version 0.1
 * @date 2026-01-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "ldr_driver.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"

/**
 * @brief 初始化光敏电阻引脚及ADC
 */
void ldr_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(LDR_GPIO_CLK, ENABLE); // 打开 ADC IO端口时钟
    GPIO_InitStructure.GPIO_Pin = LDR_GPIO_PIN;   // 配置 ADC IO 引脚模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 设置为模拟输入

    GPIO_Init(LDR_GPIO_PORT, &GPIO_InitStructure); // 初始化 ADC IO

    adcx_init();
}

/**
 * @brief 光敏电阻读取一次ADC
 * @return uint16_t 读到的模拟信号
 */
uint16_t ldr_adc_read(void)
{
    // 设置指定ADC的规则组通道，采样时间  ADC_SampleTime_13Cycles5
    // return ADC_GetValue(LDR_ADC_CHANNEL, ADC_SampleTime_55Cycles5);
    return adc_get_value(LDR_ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}

/**
 * @brief 光敏电阻读LDR_READ_TIMES次ADC取平均值
 * @return  读到的模拟信号的平均值
 */
uint16_t ldr_average_date(void)
{
    uint32_t temp_data = 0;
    for (uint8_t i = 0; i < LDR_READ_TIMES; i++)
    {
        temp_data += ldr_adc_read();
        timer_delay_ms(20);
    }

    temp_data /= LDR_READ_TIMES;
    return (uint16_t)temp_data;
}

uint16_t ldr_lux_data()
{
    float voltage = 0;
    float r = 0;
    uint16_t lux = 0;
    voltage = ldr_average_date();    // 得到平均 ADC 值（0~4095）
    voltage = voltage / 4096 * 3.3f; // 换算成实际电压（V） 12位分辨率 参考电压3.3v

    r = voltage / (3.3f - voltage) * 10000; // 分压公式：Rldr = R固定 * (Vcc - Vout) / Vout 模块固定电阻10kΩ

    lux = 40000 * pow(r, -0.6012); // 经验公式：lux = 40000 * (Rldr)^(-0.6012)

    if (lux > 999) // 上限 999 lux
        lux = 999;

    return lux;
}

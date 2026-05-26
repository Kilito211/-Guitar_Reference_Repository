/**
 * @file mq135_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief MQ-135烟雾浓度传感器驱动
 * @version 0.1
 * @date 2026-02-11
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "mq135_driver.h"

/**
 * @brief 初始化MQ-135
 */
void mq135_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(MQ135_SWITCH_SYSCTL_PERIPH_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = MQ135_SWITCH_GPIO_PIN; // PA8
    GPIO_InitStructure.GPIO_Mode = MQ135_GPIO_MODE;
#if MQ135_MODE
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#endif // MQ135_MODE
    GPIO_Init(MQ135_SWITCH_PORT, &GPIO_InitStructure);

#if MQ135_MODE
    adcx_init();
#endif // MQ135_MODE
}

#if MQ135_MODE
/**
 * @brief MQ135单次采样
 *
 * @return uint16_t 单次采样的值
 */
static uint16_t mq135_adc_read(void)
{
    return adc_get_value(MQ135_ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}
#endif // MQ135_MODE

/**
 * @brief MQ-135获取数据
 *
 * @return uint16_t 获取到的平均值
 */
uint16_t mq135_get_data(void)
{
    uint32_t temp_data = 0;

#if MQ135_MODE
    for (uint8_t i = 0; i < MQ135_READ_TIMES; i++)
    {
        temp_data += mq135_adc_read();
        timer_delay_ms(5);
    }
    temp_data /= MQ135_READ_TIMES;
#else
    temp_data = !GPIO_ReadInputDataBit(MQ135_SWITCH_PORT, MQ135_SWITCH_GPIO_PIN);
#endif // MQ135_MODE

    return temp_data;
}

/**
 * @brief MQ-135获取PPM值
 *
 * @return float
 */
float mq135_get_data_ppm(void)
{
#if MQ135_MODE
    float temp_data = 0;

    for (uint8_t i = 0; i < MQ135_READ_TIMES; i++)
    {
        temp_data += mq135_adc_read();
        timer_delay_ms(5);
    }
    temp_data /= MQ135_READ_TIMES; // 计算平均值

    float vol = (temp_data * 5 / 4096); // 计算电压
    float rs = (5 - vol) / (vol * 0.5); // 计算电阻
    float r0 = 6.64;

    float ppm = pow(11.5428 * r0 / rs, 0.6549f); // 计算ppm值

    return ppm;

#endif // MQ135_MODE
}
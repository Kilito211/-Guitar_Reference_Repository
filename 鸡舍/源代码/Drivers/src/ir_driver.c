/**
 * @file ir_driver.c
 * @author kilito_hyx (kilito.hyx@gmail.com)
 * @brief 火焰传感器驱动
 * @version 0.1
 * @date 2026-02-18
 *
 * @copyright Copyright (c) 2026
 *
 */

#include "workqueue.h"

/**
 * @brief 初始化火焰传感器
 */
void ir_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(IR_SWITCH_SYSCTL_PERIPH_CLK, ENABLE);

    GPIO_InitStructure.GPIO_Pin = IR_SWITCH_GPIO_PIN; // PA6
    GPIO_InitStructure.GPIO_Mode = IR_GPIO_MODE;
#if IR_MODE
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#endif // IR_MODE
    GPIO_Init(IR_SWITCH_PORT, &GPIO_InitStructure);

#if IR_MODE
    adcx_init();
#endif // IR_MODE
}

#if IR_MODE
/**
 * @brief 单次读取火焰传感器adc值
 *
 * @return uint16_t 读取到的电压值
 */
static uint16_t ir_adc_read(void)
{
    return adc_get_value(IR_ADC_CHANNEL, ADC_SampleTime_55Cycles5);
}
#endif // IR_MODE

uint16_t ir_fire_data()
{
#if IR_MODE
    uint32_t temp_data = 0;
    for (uint8_t i = 0; i < IR_READ_TIMES; i++)
    {
        temp_data += ir_adc_read();
        timer_delay_ms(5);
    }
    temp_data /= IR_READ_TIMES;
    return 4095 - (uint16_t)temp_data;
#else
    uint16_t temp_data;
    temp_data = !GPIO_ReadInputDataBit(IR_SWITCH_PORT, IR_SWITCH_GPIO_PIN);
    return temp_data;
#endif
}

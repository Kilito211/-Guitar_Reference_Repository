#include "adcx_driver.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_rcc.h"

/**
 * @brief ADC初始化程序
 */
void adcx_init(void)
{
    // 开启ADC时钟
    RCC_APB2PeriphClockCmd(ADC_CLK, ENABLE);

    // 配置ADC结构体
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);

    // 配置ADC结构体
    ADC_InitTypeDef ADC_InitStructure;
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;                  // 独立模式
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;              // 数据右对齐
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 软件触发
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;                 // 单次转换
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                       // 非扫描模式
    ADC_InitStructure.ADC_NbrOfChannel = 1;                             // 总通道数
    ADC_Init(ADCx, &ADC_InitStructure);                                 // 初始化ADC1

    // 开启ADCx
    ADC_Cmd(ADCx, ENABLE);

    // 进行ADC校准
    ADC_ResetCalibration(ADCx);
    while (ADC_GetResetCalibrationStatus(ADCx) == SET)
        ;
    ADC_StartCalibration(ADCx);
    while (ADC_GetCalibrationStatus(ADCx) == SET)
        ;
}

/**
 * @brief  获取ADC转换后的数据
 * @param  ADC_Channel 	选择需要采集的ADC通道
 * @param  ADC_SampleTime  选择需要采样时间
 * @retval 返回转换后的模拟信号数值
 */
u16 adc_get_value(uint8_t adc_channel, uint8_t adc_sample_time)
{
    // 配置ADC通道
    ADC_RegularChannelConfig(ADCx, adc_channel, 1, adc_sample_time);

    ADC_SoftwareStartConvCmd(ADCx, ENABLE); // 软件触发ADC转换
    while (ADC_GetFlagStatus(ADCx, ADC_FLAG_EOC) == RESET)
        ; // 读取ADC转换完成标志位
    return ADC_GetConversionValue(ADCx);
}

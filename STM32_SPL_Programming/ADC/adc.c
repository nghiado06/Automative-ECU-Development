#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"

void APB2_Config()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
}

void GPIO_Config()
{
    GPIO_InitTypeDef gpio;

    gpio.GPIO_Mode = GPIO_Mode_AIN;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(GPIOA, &gpio);
}

void ADC_Config()
{
    ADC_InitTypeDef adc;
    adc.ADC_Mode = ADC_Mode_Independent;
    adc.ADC_ScanConvMode = DISABLE;
    adc.ADC_ContinuousConvMode = ENABLE;
    adc.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    adc.ADC_DataAlign = ADC_DataAlign_Right;
    adc.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &adc);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_71Cycles5);

    ADC_Cmd(ADC1, ENABLE); // Bật ADC

    ADC_ResetCalibration(ADC1); // Reset
    while (ADC_GetResetCalibrationStatus(ADC1))
        ; // Đợi xong

    ADC_StartCalibration(ADC1); // Bắt đầu hiệu chuẩn
    while (ADC_GetCalibrationStatus(ADC1))
        ; // Đợi hoàn thành
}

uint16_t readADC()
{
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ;
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    return ADC_GetConversionValue(ADC1);
}

int main()
{
    APB2_Config();
    GPIO_Config();
    ADC_Config();

    while (1)
    {
        uint16_t value = readADC(); // Xử lý value này
    }

    return 0;
}
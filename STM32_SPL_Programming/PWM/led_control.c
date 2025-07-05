#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void PWM_Init(void)
{
    // Bật clock GPIOA và TIM2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // PA0 (TIM2_CH1) làm Alternate Function Push-Pull
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_0;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // Cấu hình Timer2
    TIM_TimeBaseInitTypeDef tim;
    tim.TIM_Prescaler = 71; // f_timer = 72MHz / (71+1) = 1MHz
    tim.TIM_Period = 999;   // PWM freq = 1MHz / (999+1) = 1kHz
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &tim);

    // Cấu hình PWM mode cho TIM2_CH1
    TIM_OCInitTypeDef oc;
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 0; // Ban đầu duty = 0%
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC1Init(TIM2, &oc);

    // Bật Timer
    TIM_Cmd(TIM2, ENABLE);
}

int main(void)
{
    PWM_Init();

    while (1)
    {
        for (int duty = 0; duty <= 1000; duty += 10)
        {
            TIM_SetCompare1(TIM2, duty); // duty = 0 → 1000
            for (volatile int i = 0; i < 10000; ++i)
                ; // delay
        }
    }
}

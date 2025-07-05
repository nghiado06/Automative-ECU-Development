#include "stm32f10x.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void PWM_Servo_Init(void)
{
    // Bật clock GPIOA và TIM2
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // PA1 (TIM2_CH2)
    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_1;
    gpio.GPIO_Mode = GPIO_Mode_AF_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpio);

    // Cấu hình Timer2 cho PWM 50Hz (20ms)
    TIM_TimeBaseInitTypeDef tim;
    tim.TIM_Prescaler = 71; // f_timer = 72MHz / (71+1) = 1MHz
    tim.TIM_Period = 19999; // 20ms chu kỳ → 1MHz / 20000 = 50Hz
    tim.TIM_CounterMode = TIM_CounterMode_Up;
    tim.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &tim);

    // PWM mode cho TIM2_CH2
    TIM_OCInitTypeDef oc;
    oc.TIM_OCMode = TIM_OCMode_PWM1;
    oc.TIM_OutputState = TIM_OutputState_Enable;
    oc.TIM_Pulse = 1500; // 1.5ms → góc trung tâm
    oc.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC2Init(TIM2, &oc);

    TIM_Cmd(TIM2, ENABLE);
}

// Chuyển góc (0–180 độ) thành CCR
uint16_t angle_to_ccr(uint8_t angle)
{
    return 1000 + (angle * 1000) / 180; // 1000 → 2000
}

int main(void)
{
    PWM_Servo_Init();

    while (1)
    {
        for (uint8_t a = 0; a <= 180; a += 10)
        {
            TIM_SetCompare2(TIM2, angle_to_ccr(a));
            for (volatile int i = 0; i < 1000000; ++i)
                ; // delay 1s
        }
    }
}

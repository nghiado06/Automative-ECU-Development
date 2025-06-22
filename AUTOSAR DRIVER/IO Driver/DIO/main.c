#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

void delay(volatile int t)
{
}

int main(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef Gpio;
    Gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    Gpio.GPIO_Pin = GPIO_Pin_13;
    Gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &Gpio);

    while (1)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13);
    }
}

#include "Dio.h"
#include "Dio_Cfg.h"
#include "stm32f10x_rcc.h"

void delay(int time)
{
    for (volatile int i = 0; i < time; i++)
        ;
}

int main()
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitTypeDef gpio;
    gpio.GPIO_Pin = GPIO_Pin_13;
    gpio.GPIO_Mode = GPIO_Mode_Out_PP;
    gpio.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpio);

    while (1)
    {
        Dio_WriteChannel(DIO_CHANNEL_C13, STD_LOW);
        delay(1000000);

        Dio_WriteChannel(DIO_CHANNEL_C13, STD_HIGH);
        delay(1000000);
    }

    return 0;
}
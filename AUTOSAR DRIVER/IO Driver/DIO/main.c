#include "Dio.h"
#include "Dio_Cfg.h"
#include "stm32f10x_rcc.h"

void delay(int time)
{
    for (int i = 0; i < time; i++)
    {
    }
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
        for (volatile int i = 0; i < 1000000; i++)
            ;

        Dio_WriteChannel(DIO_CHANNEL_C13, STD_HIGH);
        for (volatile int i = 0; i < 1000000; i++)
            ;
    }

    return 0;
}
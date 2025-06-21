#define PERIPH_BASE 0x40000000UL
#define APB2PERIPH_BASE (PERIPH_BASE + 0x10000)
#define RCC_BASE (PERIPH_BASE + 0x21000)
#define GPIOC_BASE (APB2PERIPH_BASE + 0x1000)

#define RCC_APB2ENR (*(volatile unsigned int *)(RCC_BASE + 0x18))
#define GPIOC_CRH (*(volatile unsigned int *)(GPIOC_BASE + 0x04))
#define GPIOC_ODR (*(volatile unsigned int *)(GPIOC_BASE + 0x0C))

void delay(volatile int t)
{
    for (int i = 0; i < t; i++)
        ;
}

int main(void)
{
    // Enable GPIOC clock (IOPCEN bit = 1 << 4)
    RCC_APB2ENR |= (1 << 4);

    // Configure PC13 as General purpose output push-pull, max speed 2 MHz (0b0010)
    GPIOC_CRH &= ~(0xF << 20); // Clear bits 20-23 (PC13)
    GPIOC_CRH |= (0x2 << 20);  // MODE13 = 0b10 (output 2MHz), CNF13 = 0b00 (push-pull)

    while (1)
    {
        GPIOC_ODR |= (1 << 13); // LED ON (active-low)
        delay(2000000);

        GPIOC_ODR &= ~(1 << 13); // LED OFF
        delay(2000000);
    }
}

#include "rcc.h"
#include "stm32f4xx.h"

/* Function to enable peripheral clock on AHB1 bus */
void set_ahb1_periph_clock(uint32_t periphs)
{
	//RCC->AHB1ENR |= periphs; - used macro alternative
	SET_BIT(RCC->AHB1ENR, periphs);
}

/* Function to enable peripheral clock on APB2 bus */
void set_apb2_periph_clock(uint32_t periphs)
{
	SET_BIT(RCC->APB2ENR, periphs);
}

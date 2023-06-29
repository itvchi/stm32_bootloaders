#include "systick.h"
#include "stm32f4xx.h"

void SysTick_DelayMS(uint32_t delay)
{
	/* Reload the number of clocks per millisecond */
	SysTick->LOAD = 16000-1; //(16000/16000000 = 0.001s)

	/* Clear the Current Value register */
	SysTick->VAL = 0;

	/* Select clock source and enable SysTick */
	SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);

	/* Wait 'delay' times for SysTick overflow */
	for(uint32_t i = 0; i<delay; i++)
		while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));

	/* Disable SysTick */
	CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
}

void SysTick_1hz_interrupt()
{
	/* Reload the number of clocks for 1 sec */
	SysTick->LOAD = 16000000-1; //(16000000/16000000 = 1s)

	/* Clear the Current Value register */
	SysTick->VAL = 0;

	/* Select clock source and enable SysTick */
	SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk);

	/* Enable SysTick interrupt */
	SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
}

#include "gpio.h"

void set_pin_mode(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t mode)
{
	/* Clear the relevant bits in MODER register */
	/* Set the relevant bits in MODER register */
	MODIFY_REG(GPIOx->MODER, 0x03<<(POSITION_VAL(pin)*2U), mode<<(POSITION_VAL(pin)*2U));
	/* pin - pin number in (1<<pinNumber) notation */
	/* e.g. pin = (1<<8U), MODER bits position = 16 and 17 */
	/* pin*2 != 16, because of notation */
	/* POSITION_VAL(pin) returns position of bit from (1<<pin) notation */
	/* POSITION_VAL(1<<8U) = 3 -> x<<(2*pin) = x<<16 (for 2 bit x value, shift is for 16 and 17 bit) */
}

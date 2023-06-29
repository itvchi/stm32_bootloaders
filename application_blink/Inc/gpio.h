#ifndef GPIO_H_
#define GPIO_H_

#include "stm32f4xx.h"
#include <stdint.h>

#define GPIO_INPUT_MODE			0x0 //0b00
#define GPIO_OUTPUT_MODE		0x1 //0b01
#define GPIO_ALTERNATE_MODE		0x2 //0b10
#define GPIO_ANALOG_MODE		0x3 //0b11

void set_pin_mode(GPIO_TypeDef *GPIOx, uint32_t pin, uint32_t mode);

#endif /* GPIO_H_ */

#ifndef SYSTICK_H_
#define SYSTICK_H_

#include <stdint.h>

void SysTick_DelayMS(uint32_t delay);
void SysTick_1hz_interrupt();

#endif /* SYSTICK_H_ */

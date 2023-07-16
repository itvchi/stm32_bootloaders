#ifndef RCC_H_
#define RCC_H_

#include <stdint.h>

void set_ahb1_periph_clock(uint32_t periphs);
void set_apb2_periph_clock(uint32_t periphs);

#endif /* RCC_H_ */

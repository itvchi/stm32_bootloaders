#ifndef LED_H_
#define LED_H_

#define LED1	13
#define LED2	14

#include <stdint.h>

void user_led_init();
void led_on(uint32_t led);
void led_off(uint32_t led);
void led_toggle(uint32_t led);

#endif /* LED_H_ */

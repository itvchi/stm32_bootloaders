#ifndef UART_H_
#define UART_H_

#include "stm32f4xx.h"
#include <stdint.h>

void uart_write(USART_TypeDef *USARTx, uint8_t data);
uint8_t uart_read(USART_TypeDef *USARTx);
void usart1_tx_init();
void usart1_rxtx_init();
void usart1_rxtx_interrupt_init();

#endif /* UART_H_ */

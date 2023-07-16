#include "uart.h"
#include "rcc.h"
#include "gpio.h"

/* TX pin is at PA9 - alternate function AF7 */
#define TX_PIN					(1U<<9)
/* RX pin is at PA10 - alternate function AF7 */
#define RX_PIN					(1U<<10)

#define USART1EN				(1U<<4)
#define UART_DATAWIDTH_8B		(0U<<USART_CR1_M_Pos)
#define UART_PARITY_NONE		(0U<<USART_CR1_PCE_Pos | 0U<<USART_CR1_PS_Pos)
#define UART_STOPBITS_1			(0b00<<USART_CR2_STOP_Pos)

static void uart_parameters_config(USART_TypeDef *USARTx, uint32_t dataWidth, uint32_t parity, uint32_t stopBits);
static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphClock, uint32_t baudrate);
static void set_uart_transfer_direction(USART_TypeDef *USARTx, uint32_t transferDirection);

/* To use printf, __putchar need to be implemente */
int __io_putchar(int ch)
{
	uart_write(USART1, ch);
	return ch;
}

void uart_write(USART_TypeDef *USARTx, uint8_t data)
{
	/* Make sure transmit data register is empty */
	/* Wait until TXE bit is 1 */
	while(!(USARTx->SR & USART_SR_TXE_Msk));

	/* Write value into transmit data register */
	USARTx->DR = data;
}

uint8_t uart_read(USART_TypeDef *USARTx)
{
	/* Make sure data buffer contains received data */
	/* Wait until RXNE bit is 1 */
	while(!(USARTx->SR & USART_SR_RXNE_Msk));

	/* Return data from receive register */
	return USARTx->DR & 0xFF;
}

void usart1_tx_init()
{
	/* Enable clock for TX pin GPIO port */
	set_ahb1_periph_clock(RCC_AHB1ENR_GPIOAEN);

	/* Set TX pin GPIO mode to alternate function */
	set_pin_mode(GPIOA, TX_PIN, GPIO_ALTERNATE_MODE);

	/* Set alternate function to USART */
	GPIOA->AFR[1] &= ~(0b1111<<4); //AFRH9 - zero bits
	GPIOA->AFR[1] |= (0b0111<<4); //AFRH9 - set AF7

	/* Enable clock to USART module */
	set_apb2_periph_clock(USART1EN);

	/* Configure USART parameters */
	uart_parameters_config(USART1, UART_DATAWIDTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	uart_set_baudrate(USART1, 16000000U, 115200); //periph_clok = APB2clock
	set_uart_transfer_direction(USART1, USART_CR1_TE);

	/* Enable USART */
	SET_BIT(USART1->CR1, USART_CR1_UE);
}

void usart1_rxtx_init()
{
	/* Enable clock for TX pin GPIO port */
	set_ahb1_periph_clock(RCC_AHB1ENR_GPIOAEN);

	/* Set TX pin GPIO mode to alternate function */
	set_pin_mode(GPIOA, TX_PIN, GPIO_ALTERNATE_MODE);
	/* Set alternate function to USART */
	GPIOA->AFR[1] &= ~(0b1111<<4); //AFRH9 - zero bits
	GPIOA->AFR[1] |= (0b0111<<4); //AFRH9 - set AF7

	/* Set RX pin GPIO mode to alternate function */
	set_pin_mode(GPIOA, RX_PIN, GPIO_ALTERNATE_MODE);
	/* Set alternate function to USART */
	GPIOA->AFR[1] &= ~(0b1111<<8); //AFRH10 - zero bits
	GPIOA->AFR[1] |= (0b0111<<8); //AFRH10 - set AF7

	/* Enable clock to USART module */
	set_apb2_periph_clock(USART1EN);

	/* Configure USART parameters */
	uart_parameters_config(USART1, UART_DATAWIDTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	uart_set_baudrate(USART1, 16000000U, 115200); //periph_clok = APB2clock
	set_uart_transfer_direction(USART1, USART_CR1_TE | USART_CR1_RE);

	/* Enable USART */
	SET_BIT(USART1->CR1, USART_CR1_UE);
}

void usart1_rxtx_interrupt_init()
{
	/* Enable clock for TX pin GPIO port */
	set_ahb1_periph_clock(RCC_AHB1ENR_GPIOAEN);

	/* Set TX pin GPIO mode to alternate function */
	set_pin_mode(GPIOA, TX_PIN, GPIO_ALTERNATE_MODE);
	/* Set alternate function to USART */
	GPIOA->AFR[1] &= ~(0b1111<<4); //AFRH9 - zero bits
	GPIOA->AFR[1] |= (0b0111<<4); //AFRH9 - set AF7

	/* Set RX pin GPIO mode to alternate function */
	set_pin_mode(GPIOA, RX_PIN, GPIO_ALTERNATE_MODE);
	/* Set alternate function to USART */
	GPIOA->AFR[1] &= ~(0b1111<<8); //AFRH10 - zero bits
	GPIOA->AFR[1] |= (0b0111<<8); //AFRH10 - set AF7

	/* Enable clock to USART module */
	set_apb2_periph_clock(USART1EN);

	/* Configure USART parameters */
	uart_parameters_config(USART1, UART_DATAWIDTH_8B, UART_PARITY_NONE, UART_STOPBITS_1);
	uart_set_baudrate(USART1, 16000000U, 115200); //periph_clok = APB2clock
	set_uart_transfer_direction(USART1, USART_CR1_TE | USART_CR1_RE);

	/* Enable USART */
	SET_BIT(USART1->CR1, USART_CR1_UE);

	/* Enable UART RXNE interrupt */
	SET_BIT(USART1->CR1, USART_CR1_RXNEIE);

	/* Enable UART interrupt in NVIC */
	NVIC_EnableIRQ(USART1_IRQn);
}

static void uart_parameters_config(USART_TypeDef *USARTx, uint32_t dataWidth, uint32_t parity, uint32_t stopBits)
{
	/* Clear ParitySelection, ParityControlEnable and WordLength bits, then set them from function arguments */
	MODIFY_REG(USARTx->CR1, USART_CR1_PS | USART_CR1_PCE | USART_CR1_M, parity | dataWidth);
	/* Clear StopBits, then set stop bits from function arguments */
	MODIFY_REG(USARTx->CR2, USART_CR2_STOP, stopBits);
}

static uint16_t compute_uart_div(uint32_t periphClock, uint32_t baudRate)
{
	return (periphClock + (baudRate/2U))/baudRate;
}

static void uart_set_baudrate(USART_TypeDef *USARTx, uint32_t periphClock, uint32_t baudrate)
{
	USARTx->BRR = compute_uart_div(periphClock, baudrate);
}

static void set_uart_transfer_direction(USART_TypeDef *USARTx, uint32_t transferDirection)
{
	/* Clear TransmitterEnable and ReceiverEnable bits, then set them from function arguments */
	MODIFY_REG(USARTx->CR1, USART_CR1_RE | USART_CR1_TE, transferDirection);
}

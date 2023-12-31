/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include "stm32f4xx.h"
#include <stdio.h>
#include "uart.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

extern uint32_t _bootram;
uint32_t *bootram = &_bootram;

typedef enum {
	CMD_HELLO = 0,
	CMD_START,
	CMD_PRINT_BOOT
} cmd_byte_te;

typedef enum {
	STATE_IDLE = 0,
	STATE_WAIT_4,
	STATE_WAIT_3,
	STATE_WAIT_2,
	STATE_WAIT_1,
	STATE_READY
} state_te;

volatile state_te rx_state = STATE_IDLE;
volatile uint32_t buffer[5] = { 0 };

void USART1_IRQHandler()
{
	uint8_t i;

	/* Check RXNE flag */
	if(USART1->SR & USART_SR_RXNE)
	{
		/* Clear flag by read DR register */
		cmd_byte_te byte = USART1->DR;

		if(rx_state == STATE_IDLE)
		{
			if(byte == CMD_HELLO)
			{
				buffer[0] = byte;
				rx_state = STATE_READY;
			}
			else if(byte == CMD_START)
			{
				buffer[4] = byte;
				rx_state = STATE_WAIT_4;
			}
			else if(byte == CMD_PRINT_BOOT)
			{
				buffer[0] = byte;
				rx_state = STATE_READY;
			}
		}
		else if((rx_state > STATE_IDLE) && (rx_state < STATE_READY))
		{
			for(i = 0; i < 4; i++)
				buffer[i] = buffer[i+1];
			buffer[4] = byte;
			rx_state++;
		}
	}
}



void jump_to_application(const uint32_t app_address)
{
	typedef void (*jump_fnPtr)();

	const uint32_t resetHandler = *(__IO uint32_t *)(app_address + 4); /* Address of application's Reset Handler */
	jump_fnPtr run = (jump_fnPtr)resetHandler; /* Function pointer to jump to application's Reset Handler */

	/* Deinit peripherals here */

	__set_MSP(*((__IO uint32_t*) app_address)); /* CMSIS function - set main stack pointer */
	run(); /* Jump to application */
}

extern uint32_t _sapp1;
extern uint32_t _sapp2;

int main(void)
{
	usart1_rxtx_interrupt_init(); //RX interrupt only

	printf("Program start\n");

	printf("Last reset cause: %08lX\n", RCC->CSR);
	SET_BIT(RCC->CSR, RCC_CSR_RMVF);

    while(1)
    {
    	switch(rx_state)
    	{
    	case STATE_READY:
    		switch(buffer[0])
    		{
    		case CMD_HELLO:
				printf("Hello from bootloader!\n");
    			break;
    		case CMD_START:
    			if(buffer[4] == 0x01)
    			{
    				printf("Running application!\n");
					jump_to_application((uint32_t)&_sapp1);
    			}
    			else
    				printf("Wrong app selected!\n");
    			break;
    		case CMD_PRINT_BOOT:
    				printf("Boot is: \n");
    				for(uint8_t i = 0; i < 10; i++)
    					printf("[%i] = %ld\n", i, bootram[i]);
    			break;
    		default:
    			break;
    		}
    		rx_state = STATE_IDLE;
    		break;
    	default:
    		break;
    	}
    }
}

//void configurePLL()
//{
//	/* Enable APB1 Power Interface clock - like in HAL, for USB ??? */
//	SET_BIT(RCC->APB1ENR, RCC_APB1ENR_PWREN);
//	/* Regulator voltage scaling output selection - Scale 2 mode */
//	MODIFY_REG(PWR->CR, PWR_CR_VOS, PWR_CR_VOS_1);
//
//	/* Initialize the RCC Oscillators */
//	SET_BIT(RCC->CR, RCC_CR_HSEON); /* HSE enable */
//	while(!(RCC->CR & RCC_CR_HSERDY)); /* Wait till HSE is ready */
//	CLEAR_BIT(RCC_CR, RCC_CR_PLLON); /* Disable the PLL */
//	while(!(RCC->CR, RCC_CR_PLLRDY)); /* Wait till PLL is ready */
//	/* Configure the main PLL clock source, multiplication and division factors. */
//	SET_BIT(RCC->PLLCFGR, RCC_PLLCFGR_PLLSRC_HSE); /* Set HSE as PLL source clock */
//	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLM_Msk, 15 << RCC_PLLCFGR_PLLM_Pos); /* Set PLLM */
//	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLN_Msk, 144 << RCC_PLLCFGR_PLLN_Pos); /* Set PLLN */
//	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLP_Msk, RCC_PLLCFGR_PLLP_0); /* Set PLLP to DIV4 */
//	MODIFY_REG(RCC->PLLCFGR, RCC_PLLCFGR_PLLQ_Msk, 5 << RCC_PLLCFGR_PLLQ_Pos); /* Set PLLQ */
//	SET_BIT(RCC_CR, RCC_CR_PLLON); /* Enable the PLL */
//	while(!(RCC->CR, RCC_CR_PLLRDY)); /* Wait till PLL is ready */
//}

/*
 *	UART Implementation with GPIO ports and GP Timers
 *		1 start bit, 8 data bits, even parity, 1 stop bit
 *		Tx:	  PA5 -	CN10 p11
 *		Rx:	  PA6 - CN10 p13
 *		GND:  	  - CN10 p9
 *		5V:   	  -	CN7  p18
 *
 */

#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> // should i be using inttypes.h ?
#include <stdlib.h>
#include <string.h>

#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_dma.h"


#define uart_reg UART4
#define uart_irqn UART4_IRQn
#define pin_gpio GPIOC
#define dma_instance DMA1
#define dma_tx_irqn DMA1_Channel1_IRQn
#define tx_pin LL_GPIO_PIN_10 // CN7 pin 1
#define tx_dma LL_DMA_CHANNEL_1
#define rx_pin LL_GPIO_PIN_11 // CN7 pin 2
#define rx_dma LL_DMA_CHANNEL_2

void UART_Setup(void);
ErrorStatus UART_TransmitMessageDMA(const char *buffer);
ErrorStatus UART_TransmitMessageAsync(const char *buffer);

// Interrupt handlers
void UART4_IRQHandler(void);
void DMA1_CH1_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif

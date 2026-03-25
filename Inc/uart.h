#ifndef __UART_H
#define __UART_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h> // should i be using inttypes.h ?

#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_gpio.h"

#define tx_timer TIM3
#define tx_irqn TIM3_IRQn
#define tx_busclk LL_APB1_GRP1_PERIPH_TIM3
#define tx_pin LL_GPIO_PIN_5
#define rx_timer TIM4
#define rx_irqn TIM4_IRQn
#define rx_busclk LL_APB1_GRP1_PERIPH_TIM4
#define rx_pin LL_GPIO_PIN_6

enum packet_stage {
	NONE, // not transmitting anything
	START, // sending the start frame
	DATA, // sending data
	PARITY,
	STOP
};

void UART_Setup(void);

void UART_TransmitMessageAsync(void *buffer, uint8_t length);
void UART_RecvMessageAsync(void *buffer, uint8_t length);

// Interrupt handlers
void TIM3_IRQHandler(void);
void TIM4_IRQHandler(void);

#ifdef __cplusplus
}
#endif

#endif

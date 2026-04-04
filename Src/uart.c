#include "uart.h"


void GPIO_Setup();

// eventually can be variable or even set by the other transmitter
static const uint32_t BAUD_RATE = 9600;


void UART_Setup() {
	GPIO_Setup();

	NVIC_SetPriority(uart_irqn, 0);
	NVIC_EnableIRQ(uart_irqn);

	LL_USART_InitTypeDef initStruct = {};
	LL_USART_StructInit(&initStruct);
	initStruct.BaudRate = BAUD_RATE;
	initStruct.DataWidth = LL_USART_DATAWIDTH_8B;
	LL_USART_ClockInitTypeDef clkInitStruct = {};
	LL_USART_ClockStructInit(&clkInitStruct);

	LL_USART_Init(uart_reg, &initStruct);
	LL_USART_ClockInit(uart_reg, &clkInitStruct);
	// LL_USART_SetAutoBaudRateMode();
	// LL_USART_EnableDirectionRx(usart_reg);
	LL_USART_Enable(uart_reg);
	// LL_USART_SetTransferDirection(uart_reg, LL_USART_DIRECTION_TX); // LL_USART_DIRECTION_TX_RX
	// DMA setup goes here
}

void GPIO_Setup() {
	LL_GPIO_InitTypeDef initStruct = {
			tx_pin,
			LL_GPIO_MODE_ALTERNATE,
			LL_GPIO_OUTPUT_PUSHPULL,
			LL_GPIO_SPEED_FREQ_MEDIUM,
			LL_GPIO_PULL_UP,
			LL_GPIO_AF_5,
	};
	LL_GPIO_Init(pin_gpio, &initStruct);

	// PA6: Rx
	initStruct.Pin = rx_pin;
	initStruct.Mode = LL_GPIO_MODE_INPUT;
	LL_GPIO_Init(pin_gpio, &initStruct);
}


/**
  * @brief  Send null-terminated message (array of characters) via UART
  * @param  buffer message to send
  * @retval None
  */
const char *tx_buf;
char *tx_seek = "\0";
void UART_TransmitMessageAsync(const char *buffer) {
	size_t len = strlen(buffer) + 1;
	tx_buf = memcpy(malloc(len), buffer, len);
	tx_seek = tx_buf;
	LL_USART_TransmitData8(uart_reg, *tx_seek);
	LL_USART_EnableDirectionTx(uart_reg);
	LL_USART_EnableIT_TC(uart_reg);
}


void UART4_IRQHandler(void) {
	if (LL_USART_IsActiveFlag_TC(uart_reg)) {
		if (*tx_seek != 0x00) {
			++tx_seek;
			LL_USART_TransmitData8(uart_reg, *tx_seek);
		} else { // stop transmission
			LL_USART_DisableDirectionTx(uart_reg);
			LL_USART_DisableIT_TC(uart_reg);
			free(tx_buf);
		}
		LL_USART_ClearFlag_TC(uart_reg);
	}
}



#include "uart.h"


void GPIO_Setup();
void Timer_Setup();
void Set_BaudRate(TIM_TypeDef *timer, uint32_t baud);

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
	LL_USART_Enable(uart_reg);
	LL_USART_EnableDirectionTx(uart_reg);
	// LL_USART_EnableDirectionRx(usart_reg);
	LL_USART_SetTransferDirection(uart_reg, LL_USART_DIRECTION_TX); // LL_USART_DIRECTION_TX_RX
	// DMA setup goes here
	// TE bit for idle first transmission
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


// probably should return that ErrorStatus struct the other functions do
// if you're using void then you have to consider the size of different data types in bufsize
// or leave it up to the caller ?
void UART_TransmitMessageAsync(void *buffer, uint8_t length) {
	/*
	tx_buf = buffer; // i guess you have to pray nobody messes with your buffer while you're transmitting ? maybe copy it instead ?
	tx_seek = tx_buf;
	tx_len = length;
	tx_stage = START;
	LL_TIM_GenerateEvent_UPDATE(rx_timer);
	LL_TIM_EnableCounter(tx_timer);
	*/
}

void UART_TransmitByte(uint8_t b) {
	LL_USART_TransmitData8(uart_reg, b);
}


// probably an interrupt actually
void UART_RecvMessageAsync(void *buffer, uint8_t length) {

}


void UART4_IRQHandler(void) {
}



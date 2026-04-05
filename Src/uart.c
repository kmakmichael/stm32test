#include "uart.h"


void GPIO_Setup(void);
void DMA_Setup(void);

// eventually can be variable or even set by the other transmitter
static const uint32_t BAUD_RATE = 9600;


void UART_Setup() {
	GPIO_Setup();
	DMA_Setup();

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
	LL_USART_EnableDMAReq_TX(uart_reg);
	// LL_USART_SetAutoBaudRateMode();
	// LL_USART_EnableDirectionRx(usart_reg);
	LL_USART_EnableDirectionTx(uart_reg);
	LL_USART_Enable(uart_reg);
	// LL_USART_SetTransferDirection(uart_reg, LL_USART_DIRECTION_TX); // LL_USART_DIRECTION_TX_RX
	// DMA setup goes here
}

void DMA_Setup() {
	NVIC_SetPriority(dma_tx_irqn, 0);
	NVIC_EnableIRQ(dma_tx_irqn);

	LL_DMA_InitTypeDef dmaInitStruct = {};
	LL_DMA_StructInit(&dmaInitStruct);
	dmaInitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
	// memory to copy from
	dmaInitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_BYTE;
	dmaInitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
	// peripheral (uart)
	dmaInitStruct.PeriphOrM2MSrcAddress = (uint32_t) &uart_reg->TDR; // uart4_tdr
	dmaInitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_BYTE;
	dmaInitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
	dmaInitStruct.PeriphRequest = LL_DMAMUX_REQ_UART4_TX;

	LL_DMA_Init(dma_instance, tx_dma, &dmaInitStruct);
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
uint8_t transmitting = 0;
ErrorStatus UART_TransmitMessageDMA(const char *buffer) {
	while (transmitting);
	LL_DMA_SetMemoryAddress(dma_instance, tx_dma, (uint32_t) buffer);
	LL_DMA_SetDataLength(dma_instance, tx_dma, strlen(buffer)+1);
	LL_DMA_EnableIT_TC(dma_instance, tx_dma);
	LL_DMA_EnableIT_TE(dma_instance, tx_dma);
	LL_USART_ClearFlag_TC(uart_reg);
	LL_USART_EnableIT_TC(uart_reg);
	LL_DMA_EnableChannel(dma_instance, tx_dma);
	LL_USART_RequestTxDataFlush(uart_reg);
	transmitting = 1;
	while (transmitting);
	return SUCCESS;
}

/**
  * @brief  Send null-terminated message (array of characters) via UART
  * @param  buffer message to send
  * @retval None
  */
const char *tx_buf;
char *tx_seek = '\0';
ErrorStatus UART_TransmitMessageAsync(const char *buffer) {
	if (transmitting) {
		return ERROR; // still transmitting
	}
	uint8_t len = strlen(buffer) + 1;
	tx_buf = memcpy(malloc(len), buffer, len);
	LL_DMA_SetMemoryAddress(dma_instance, tx_dma, (uint32_t) tx_buf);
	LL_DMA_SetDataLength(dma_instance, tx_dma, len);
	LL_DMA_EnableIT_TC(dma_instance, tx_dma);
	LL_DMA_EnableIT_TE(dma_instance, tx_dma);
	LL_USART_ClearFlag_TC(uart_reg);
	LL_USART_EnableIT_TC(uart_reg);
	LL_DMA_EnableChannel(dma_instance, tx_dma);
	LL_USART_RequestTxDataFlush(uart_reg);
	transmitting = 1;
	return SUCCESS;
}


void DMA1_CH1_IRQHandler(void) {
	if (LL_DMA_IsActiveFlag_TC1(dma_instance)) {
		LL_DMA_ClearFlag_TC1(dma_instance);
		LL_DMA_DisableChannel(dma_instance, tx_dma);
		LL_DMA_DisableIT_TC(dma_instance, tx_dma);
		LL_DMA_DisableIT_TE(dma_instance, tx_dma);
	}
	if (LL_DMA_IsActiveFlag_TE1(dma_instance)) {
		while (1) {
			// whoops
		}
	}
}

void UART4_IRQHandler(void) {
	if (LL_USART_IsActiveFlag_TC(uart_reg)) {
		// done transmitting
		transmitting = 0;
		LL_USART_ClearFlag_TC(uart_reg);
		LL_USART_DisableIT_TC(uart_reg);
	}
}



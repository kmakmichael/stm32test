#include "uart.h"

static const uint32_t BAUD_RATE = 9600;

// plans: fill buffer and walk buf_ptr along, use
// queue_size to track whats left to write

// static const uint8_t BUF_SIZE = 16;
// uint8_t buf[BUF_SIZE];
// unit8_t *buf_ptr;
uint8_t queue_size = 16;

// for now: send this on repeat
uint8_t sending = 'a';

void UART_Setup() {
	Timer_Setup();
	GPIO_Setup();

	LL_TIM_EnableCounter(tx_timer);
	LL_TIM_EnableCounter(rx_timer);

	LL_GPIO_TogglePin(GPIOA, rx_pin);
}

void Timer_Setup() {

	LL_TIM_InitTypeDef TIM_InitStruct;
	LL_TIM_StructInit(&TIM_InitStruct);

	// TIM3: Tx
	NVIC_SetPriority(tx_irqn, 0);
	NVIC_EnableIRQ(tx_irqn);
	LL_TIM_Init(tx_timer, &TIM_InitStruct);
	Set_BaudRate(tx_timer, BAUD_RATE);
	LL_TIM_EnableARRPreload(tx_timer);
	LL_TIM_EnableIT_UPDATE(tx_timer);
	LL_TIM_ClearFlag_UPDATE(tx_timer);

	// TIM4: Rx
	NVIC_SetPriority(rx_irqn, 0);
	NVIC_EnableIRQ(rx_irqn);
	LL_TIM_Init(rx_timer, &TIM_InitStruct);
	Set_BaudRate(rx_timer, BAUD_RATE);
	LL_TIM_EnableARRPreload(rx_timer);
	LL_TIM_EnableIT_UPDATE(rx_timer);
	LL_TIM_ClearFlag_UPDATE(rx_timer);
}

void GPIO_Setup() {
	LL_GPIO_InitTypeDef initStruct = {
			tx_pin,
			LL_GPIO_MODE_OUTPUT,
			LL_GPIO_OUTPUT_PUSHPULL,
			LL_GPIO_SPEED_FREQ_MEDIUM,
			LL_GPIO_PULL_UP,
			LL_GPIO_AF_0,
	};
	LL_GPIO_Init(GPIOA, &initStruct);

	// PA6: Rx
	initStruct.Pin = rx_pin;
	initStruct.Mode = LL_GPIO_MODE_INPUT;
	LL_GPIO_Init(GPIOA, &initStruct);
}


/*
 * Set timers to a certain baud rate
 * 	(is there a "right" way to balance prescaler and autoreload?)
 */
void Set_BaudRate(TIM_TypeDef *timer, uint32_t baud) {
	uint32_t prescale =__LL_TIM_CALC_PSC(SystemCoreClock, 1000 * baud);
	uint32_t autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, prescale, baud);
	LL_TIM_SetPrescaler(timer, prescale);
	LL_TIM_SetAutoReload(timer, autoreload);
	LL_TIM_GenerateEvent_UPDATE(timer);
}


/*
 * Transmit sequence: 1 clock of LOW, then 8 of data, 1 parity, 2 stop LOW
 */
// TODO: structify this ?
uint8_t tx_mask = 0x01;
uint8_t tx_parity = 0;
enum packet_stage tx_stage = NONE;
/*
 * TIM3 Interrupt Handler: Tx cycle
 */
void TIM3_IRQHandler(void) {
	switch (tx_stage) {
	case NONE:
		LL_GPIO_SetOutputPin(GPIOA, tx_pin); // just make sure we're on HI
		if (queue_size > 0) {
			tx_stage = START;
		}
		break;
	case START: // 1 cycle of LO
		LL_GPIO_ResetOutputPin(GPIOA, tx_pin);
		tx_mask = 0x01;
		tx_stage = DATA;
		tx_parity = 0x0;
		break;
	case DATA: // 8 cycles of bits
		uint8_t bit = sending & tx_mask;
		(bit) ? LL_GPIO_SetOutputPin(GPIOA, tx_pin) : LL_GPIO_ResetOutputPin(GPIOA, tx_pin);
		if (bit) {
			++tx_parity;
		}
		tx_mask <<= 1;
		if (tx_mask == 0) {
			tx_stage = PARITY;
		}
		break;
	case PARITY: // 1 cycle of parity
		(tx_parity & 0x01) ? LL_GPIO_ResetOutputPin(GPIOA, tx_pin) : LL_GPIO_SetOutputPin(GPIOA, tx_pin);
		tx_stage = STOP;
		break;
	case STOP: // 2 cycles of HI (1 here, 1 as NONE)
		LL_GPIO_SetOutputPin(GPIOA, tx_pin);
		tx_stage = NONE;
		break;
	default: // some kind of error ? maybe jump into an error handler ?
		break;
	}
	LL_TIM_ClearFlag_UPDATE(TIM3);
}


uint8_t rx_mask = 0x01;
/*
 * TIM4 Interrupt Handler: Rx cycle
 */
void TIM4_IRQHandler(void) {
	LL_GPIO_TogglePin(GPIOA, rx_pin);
	LL_TIM_ClearFlag_UPDATE(TIM4);
}


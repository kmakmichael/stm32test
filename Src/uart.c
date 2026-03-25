#include "uart.h"


void GPIO_Setup();
void Timer_Setup();
void Set_BaudRate(TIM_TypeDef *timer, uint32_t baud);

// eventually can be variable or even set by the other transmitter
static const uint32_t BAUD_RATE = 9600;

/* transmission buffer */
void *tx_buf;
uint8_t *tx_seek;
uint8_t tx_len = 0; // size_t on the system is probably 32 but whatever, limit it to 8.

/* recv buffer */
void *rx_buf;
uint8_t *rx_seek;
uint8_t rx_len = 0;

// for now: send this on repeat
uint8_t sending = 0b01010101;

void UART_Setup() {
	Timer_Setup();
	GPIO_Setup();
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

	// PA7: debug signal
	initStruct.Pin = LL_GPIO_PIN_7;
	initStruct.Mode = LL_GPIO_MODE_OUTPUT;
	LL_GPIO_Init(GPIOA, &initStruct);
}


// probably should return that ErrorStatus struct the other functions do
// if you're using void then you have to consider the size of different data types in bufsize
// or leave it up to the caller ?
void UART_TransmitMessageAsync(void *buffer, uint8_t length) {
	tx_buf = buffer; // i guess you have to pray nobody messes with your buffer while you're transmitting ? maybe copy it instead ?
	tx_seek = tx_buf;
	tx_len = length;

	LL_TIM_EnableCounter(tx_timer);
}

void UART_RecvMessageAsync(void *buffer, uint8_t length) {

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
	if (tx_stage == NONE) {
		LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_7);
	} else {
		LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_7);
	}
	switch (tx_stage) {
	case NONE:
		LL_GPIO_SetOutputPin(GPIOA, tx_pin); // just make sure we're on HI
		if (tx_seek > tx_buf + tx_len) { // if we're done with our data, stop
			tx_len = 0;
			LL_TIM_DisableCounter(tx_timer);
			// do something with tx_seek ?
			break; // probably not necessary but it feels safer to wait a cycle before checking for more data
		}
		if (tx_len > 0) {
			tx_stage = START;
		}
		break;
	case START: // 1 cycle of LO
		LL_GPIO_ResetOutputPin(GPIOA, tx_pin);
		tx_mask = 0x01;
		tx_parity = 0x00;
		tx_stage = DATA;
		break;
	case DATA: // 8 cycles of bits
		uint8_t bit = *tx_seek & tx_mask;
		(bit) ? LL_GPIO_SetOutputPin(GPIOA, tx_pin) : LL_GPIO_ResetOutputPin(GPIOA, tx_pin); // one : zero
		if (bit) {
			++tx_parity;
		}
		tx_mask <<= 1;
		if (tx_mask == 0) {
			tx_stage = PARITY;
		}
		break;
	case PARITY: // 1 cycle of parity
		(tx_parity & 0x01) ? LL_GPIO_SetOutputPin(GPIOA, tx_pin) : LL_GPIO_ResetOutputPin(GPIOA, tx_pin); // odd : even
		// walk the buffer pointer along
		++tx_seek;
		tx_parity = 0xFF;
		tx_stage = STOP;
		break;
	case STOP: // 2 cycles of HI (1 as STOP, 1 as NONE)
		LL_GPIO_SetOutputPin(GPIOA, tx_pin);
		tx_stage = NONE;
		break;
	default: // some kind of error ? maybe jump into an error handler ?
		break;
	}
	LL_TIM_ClearFlag_UPDATE(TIM3);
}


uint8_t rx_mask = 0x01;
enum packet_stage rx_stage = NONE;
void *rx_bufptr;
/*
 * TIM4 Interrupt Handler: Rx cycle
 */
void TIM4_IRQHandler(void) {
	LL_GPIO_TogglePin(GPIOA, rx_pin);
	LL_TIM_ClearFlag_UPDATE(TIM4);
}


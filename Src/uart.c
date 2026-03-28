#include "uart.h"


void GPIO_Setup();
void Timer_Setup();
void Set_BaudRate(TIM_TypeDef *timer, uint32_t baud);

// eventually can be variable or even set by the other transmitter
static const uint32_t BAUD_RATE = 9600;

// TODO: structify this ?
/* transmission buffer */
void *tx_buf;
uint8_t *tx_seek;
uint8_t tx_len = 0; // size_t on the system is probably 32 but whatever, limit it to 8.
uint8_t tx_mask = 0x01;
uint8_t tx_parity = 0;
enum packet_stage tx_stage = SETUP;

/* recv buffer */
void *rx_buf;
uint8_t *rx_seek;
uint8_t rx_len = 0;
uint8_t rx_bit = 0;
uint8_t rx_parity = 0;
enum packet_stage rx_stage = SETUP;

/*
 * Transmit sequence: 1 clock of LOW, then 8 of data, 1 parity, 1 stop HI
 */

void UART_Setup() {
	Timer_Setup();
	GPIO_Setup();
	rx_buf = calloc(256, 1);
	rx_seek = rx_buf;
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

	// EXTI for Rx pin
	LL_EXTI_EnableIT_0_31(rx_exti);
	LL_EXTI_EnableFallingTrig_0_31(rx_exti);
	NVIC_SetPriority(EXTI9_5_IRQn, 0);
	NVIC_EnableIRQ(EXTI9_5_IRQn);

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
	tx_stage = START;
	LL_TIM_GenerateEvent_UPDATE(rx_timer);
	LL_TIM_EnableCounter(tx_timer);
}

// probably an interrupt actually
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
}


/*
 * TIM3 Interrupt Handler: Tx cycle
 */
void TIM3_IRQHandler(void) {
	switch (tx_stage) {
	case SETUP:
		LL_GPIO_SetOutputPin(GPIOA, tx_pin); // just make sure we're on HI
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
	case STOP: // 1 cycle HI
		LL_GPIO_SetOutputPin(GPIOA, tx_pin);
		if (tx_seek > tx_buf + tx_len) { // if we're done with our data, stop
			tx_len = 0;
			LL_TIM_DisableCounter(tx_timer);
		}
		tx_stage = START;
		break;
	default: // some kind of error ? maybe jump into an error handler ?
		break;
	}
	LL_TIM_ClearFlag_UPDATE(TIM3);
}


/*
 * TIM4 Interrupt Handler: Rx cycle
 * currently operating on the assumption of 8 data bits plus parity
 */
void TIM4_IRQHandler(void) {
	switch (rx_stage) {
	case SETUP: // use this stage to wait a half-cycle for rx reads
		Set_BaudRate(rx_timer, BAUD_RATE);
		rx_stage = START;
		break;
	case START:
		rx_bit = 0;
		rx_parity = 0x00;
		rx_stage = DATA;
		break;
	case DATA:
		uint8_t rx_data = LL_GPIO_IsInputPinSet(GPIOA, rx_pin) << rx_bit;
		*rx_seek |= rx_data;
		if (rx_data) {
			++rx_parity;
		}
		++rx_bit;
		if (rx_bit > 7) {
			rx_stage = PARITY;
		}
		break;
	case PARITY:
		// check it or whatever
		rx_stage = STOP;
		break;
	case STOP:
		rx_stage = SETUP;
		LL_TIM_DisableCounter(rx_timer);
		break;
	default:
		break;

	}
	LL_TIM_ClearFlag_UPDATE(TIM4);
}


/*
 *	Rx detected
 */
void EXTI9_5_IRQHandler(void) {
	// some conditional to check if its the rx pin would be thorough
	if(!LL_TIM_IsEnabledCounter(rx_timer)) { // start up the rx clock
		rx_stage = SETUP;
		rx_seek = rx_buf;
		Set_BaudRate(rx_timer, 2*BAUD_RATE);
		LL_TIM_GenerateEvent_UPDATE(rx_timer);
		LL_TIM_EnableCounter(rx_timer);
	} else { // rx'ing, so ignore

	}
	LL_EXTI_ClearFlag_0_31(rx_exti);
}


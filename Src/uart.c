#include "uart.h"

static const uint32_t BAUD_RATE = 2; //9600;


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
	TIM_InitStruct.Prescaler = __LL_TIM_CALC_PSC(SystemCoreClock, BAUD_RATE);
	TIM_InitStruct.Autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, TIM_InitStruct.Prescaler, BAUD_RATE);

	// TIM3: Tx
	NVIC_EnableIRQ(tx_irqn);
	LL_TIM_Init(tx_timer, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(tx_timer);
	LL_TIM_EnableIT_UPDATE(tx_timer);
	LL_TIM_ClearFlag_UPDATE(tx_timer);

	// TIM4: Rx
	NVIC_EnableIRQ(rx_irqn);
	LL_TIM_Init(rx_timer, &TIM_InitStruct);
	LL_TIM_EnableARRPreload(rx_timer);
	LL_TIM_EnableIT_UPDATE(rx_timer);
	LL_TIM_ClearFlag_UPDATE(rx_timer);
}

void GPIO_Setup() {
	LL_GPIO_InitTypeDef initStruct = {
			tx_pin,
			LL_GPIO_MODE_OUTPUT,
			LL_GPIO_OUTPUT_PUSHPULL,
			LL_GPIO_SPEED_FREQ_LOW,
			LL_GPIO_PULL_UP,
			LL_GPIO_AF_0,
	};
	LL_GPIO_Init(GPIOA, &initStruct);

	// PA6: Rx
	initStruct.Pin = rx_pin;
	// initStruct.Mode = LL_GPIO_MODE_INPUT;
	LL_GPIO_Init(GPIOA, &initStruct);
}


void Set_BaudRate(TIM_TypeDef *timer, uint32_t baud) {
	uint32_t prescale =__LL_TIM_CALC_PSC(SystemCoreClock, baud);
	uint32_t autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, prescale, baud);
	LL_TIM_SetPrescaler(timer, prescale);
	LL_TIM_SetAutoReload(timer, autoreload);
	LL_TIM_GenerateEvent_UPDATE(timer);
}


/*
 * TIM3 Interrupt Handler: Tx cycle
 */
void TIM3_IRQHandler(void) {
	LL_GPIO_TogglePin(GPIOA, tx_pin);
	LL_TIM_ClearFlag_UPDATE(TIM3);
}

/*
 * TIM4 Interrupt Handler: Rx cycle
 */
void TIM4_IRQHandler(void) {
	LL_GPIO_TogglePin(GPIOA, rx_pin);
	LL_TIM_ClearFlag_UPDATE(TIM4);
}

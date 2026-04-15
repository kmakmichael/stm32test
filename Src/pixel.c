#include "pixel.h"


Pixel pixels[NUM_PIXELS];
Pixel test_px;


void PIX_Init(void) {

}

void PIX_Write(void) {

}


/*
 * from uart timer code. is this accurate enough ?
void Set_BaudRate(TIM_TypeDef *timer, uint32_t baud) {
  uint32_t prescale =__LL_TIM_CALC_PSC(SystemCoreClock, 1000 * baud);
  uint32_t autoreload = __LL_TIM_CALC_ARR(SystemCoreClock, prescale, baud);
  LL_TIM_SetPrescaler(timer, prescale);
  LL_TIM_SetAutoReload(timer, autoreload);
}
*/

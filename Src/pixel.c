#include "pixel.h"


void PIX_GPIO_Setup(void);
void PIX_HRTIM_Setup(void);

Pixel pixels[NUM_PIXELS];
uint8_t *seek;
uint8_t current_bit;


ErrorStatus PIX_Init(void) {
  PIX_GPIO_Setup();
  PIX_HRTIM_Setup();
  for (size_t p = 0; p < NUM_PIXELS; p++) {
    pixels[p].Green = 0xff;
    pixels[p].Red = 0x11;
    pixels[p].Blue = 0x99;
  }
  current_bit = 0x0;
  seek = &pixels[0].Green;
  return SUCCESS;
}

void PIX_GPIO_Setup() {
  LL_GPIO_InitTypeDef initStruct = {
        PIX_PINOUT,
        LL_GPIO_MODE_OUTPUT,
        LL_GPIO_OUTPUT_PUSHPULL,
        LL_GPIO_SPEED_FREQ_HIGH, // i don't think VERYHIGH is necessary
        LL_GPIO_PULL_DOWN,
        LL_GPIO_AF_13,
    };
    LL_GPIO_Init(PIX_GPIO, &initStruct);
}

void PIX_HRTIM_Setup() {

}


// gpio high speed rises and falls within 6ns
// gpio med spd 11 ns
// flash programming time is 83.83 ms ??
// cycle time: 1250ns +/- 600ns (each stage is +/- 150ns)
// -> 0.8 MHz
// reset: LO for >=50us
// 50ns -> 20MHz
ErrorStatus PIX_Write(void) {
  // check if we're already writing
  if (0x0) {
    return ERROR;
  }
  // set the variables up
  // boot up HRTIM
  if (0x0) { // 1
    /*
     * T1H: 800ns
     * T1L: 450ns
     */

  } else { // 0
    /*
     * T0H: 400ns
     * T0L: 850ns
     */

  }
  LL_GPIO_TogglePin(PIX_GPIO, PIX_PINOUT);
  return SUCCESS;
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

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
        LL_GPIO_MODE_ALTERNATE,
        LL_GPIO_OUTPUT_PUSHPULL,
        LL_GPIO_SPEED_FREQ_HIGH, // i don't think VERYHIGH is necessary
        LL_GPIO_PULL_DOWN,
        LL_GPIO_AF_13,
    };
    LL_GPIO_Init(PIX_GPIO, &initStruct);
}



void PIX_HRTIM_Setup() {
  LL_HRTIM_ConfigDLLCalibration(PIX_HRTIM, LL_HRTIM_DLLCALIBRATION_MODE_CONTINUOUS, LL_HRTIM_DLLCALIBRATION_RATE_3);
  LL_HRTIM_TIM_SetCounterMode(PIX_HRTIM, LL_HRTIM_TIMER_MASTER, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetCounterMode(PIX_HRTIM, PIX_TIMER, LL_HRTIM_MODE_CONTINUOUS);
  // set Timer A cycle to 1.25us
  LL_HRTIM_TIM_SetPrescaler(PIX_HRTIM, LL_HRTIM_TIMER_MASTER, LL_HRTIM_PRESCALERRATIO_MUL32); // resolution of 184ps (170MHz * 32)
  LL_HRTIM_TIM_SetPrescaler(PIX_HRTIM, PIX_TIMER, LL_HRTIM_PRESCALERRATIO_MUL32); // might not be necessary ?
  uint32_t period = 1250000 / 184;
  LL_HRTIM_TIM_SetPeriod(PIX_HRTIM, PIX_TIMER, period);

  NVIC_SetPriority(PIX_IRQN, 0);
  NVIC_EnableIRQ(PIX_IRQN);
  LL_HRTIM_EnableIT_RST(PIX_HRTIM, PIX_TIMER);

  /*
  // compare events (at 400us & 800us)
  LL_HRTIM_TIM_SetCompare1(PIX_HRTIM, PIX_TIMER, period * 400 / 1250);
  LL_HRTIM_TIM_SetCompare2(PIX_HRTIM, PIX_TIMER, period * 800 / 1250);
  LL_HRTIM_TIM_SetComp1Mode(PIX_HRTIM, PIX_TIMER, LL_HRTIM_GTCMP1_GREATER);

  // output setup
  LL_HRTIM_OUT_SetPolarity(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetIdleMode(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetOutputSetSrc(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUTPUTSET_TIMCMP1);
  LL_HRTIM_OUT_SetOutputResetSrc(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUTPUTRESET_UPDATE);
  */

  while(!LL_HRTIM_IsActiveFlag_DLLRDY(PIX_HRTIM)) { } // wait for ready flag

  // enable the master timer
  LL_HRTIM_TIM_CounterEnable(PIX_HRTIM, LL_HRTIM_TIMER_MASTER);

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
  LL_HRTIM_CounterReset(PIX_HRTIM, PIX_TIMER);
  LL_HRTIM_TIM_CounterEnable(PIX_HRTIM, PIX_TIMER);
  LL_HRTIM_EnableOutput(PIX_HRTIM, PIX_TIMOUT);
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
  return SUCCESS;
}

void HRTIM_TIMA_IRQn_IRQHandler() {
  // look for the next one
  LL_HRTIM_ClearFlag_RST(PIX_HRTIM, PIX_TIMER);
}

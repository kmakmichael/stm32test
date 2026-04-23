#include "pixel.h"


void PIX_GPIO_Setup(void);
void PIX_HRTIM_Setup(void);
void PIX_DMA_Setup(void);
void Ghost(Pixel pix, size_t idx);

Pixel pixels[NUM_PIXELS];
uint16_t pixels_ghost[NUM_PIXELS * 24];
// unneeded ?
PixelCommsState state;
uint8_t rest_timer;


ErrorStatus PIX_Init(void) {
  PIX_GPIO_Setup();
  PIX_DMA_Setup();
  PIX_HRTIM_Setup();
  for (size_t p = 0; p < NUM_PIXELS; p++) {
    if (p % 2) {
      pixels[p].Green = 0x00;
      pixels[p].Red = 0xff;
      pixels[p].Blue = 0x00;
    } else {
      pixels[p].Green = 0xaa;
      pixels[p].Red = 0xaa;
      pixels[p].Blue = 0xaa;
    }
    Ghost(pixels[p], p);
  }
  rest_timer = 0;
  return SUCCESS;
}



void PIX_GPIO_Setup() {
  LL_GPIO_InitTypeDef initStruct = {
    PIX_PINOUT,
    LL_GPIO_MODE_ALTERNATE,
    LL_GPIO_OUTPUT_PUSHPULL,
    LL_GPIO_SPEED_FREQ_HIGH,
    LL_GPIO_PULL_DOWN,
    LL_GPIO_AF_13,
  };
  LL_GPIO_Init(PIX_GPIO, &initStruct);

  initStruct.Pin = LL_GPIO_PIN_9;
  initStruct.Mode = LL_GPIO_MODE_OUTPUT;
  LL_GPIO_Init(PIX_GPIO, &initStruct);
}



void PIX_HRTIM_Setup() {
  LL_HRTIM_ConfigDLLCalibration(PIX_HRTIM, LL_HRTIM_DLLCALIBRATION_MODE_CONTINUOUS, LL_HRTIM_DLLCALIBRATION_RATE_3);
  LL_HRTIM_TIM_SetCounterMode(PIX_HRTIM, LL_HRTIM_TIMER_MASTER, LL_HRTIM_MODE_CONTINUOUS);

  LL_HRTIM_TIM_SetCounterMode(PIX_HRTIM, PIX_TIMER, LL_HRTIM_MODE_CONTINUOUS);
  LL_HRTIM_TIM_SetUpdateTrig(PIX_HRTIM, PIX_TIMER, LL_HRTIM_UPDATETRIG_RESET);
  LL_HRTIM_TIM_EnablePreload(PIX_HRTIM, PIX_TIMER);
  LL_HRTIM_TIM_SetCompare1(PIX_HRTIM, PIX_TIMER, T1H);

  // set Timer A cycle to 1.25us
  LL_HRTIM_TIM_SetPrescaler(PIX_HRTIM, LL_HRTIM_TIMER_MASTER, LL_HRTIM_PRESCALERRATIO_MUL32); // resolution of 184ps (170MHz * 32)
  LL_HRTIM_TIM_SetPeriod(PIX_HRTIM, PIX_TIMER, PERIOD);
  LL_HRTIM_EnableDMAReq_RST(PIX_HRTIM, PIX_TIMER);

  NVIC_SetPriority(PIX_IRQN, 0);
  NVIC_EnableIRQ(PIX_IRQN);
  LL_HRTIM_EnableIT_REP(PIX_HRTIM, PIX_TIMER);

  // output setup
  LL_HRTIM_OUT_SetPolarity(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUT_POSITIVE_POLARITY);
  LL_HRTIM_OUT_SetIdleMode(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUT_NO_IDLE);
  LL_HRTIM_OUT_SetIdleLevel(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUT_IDLELEVEL_INACTIVE);
  LL_HRTIM_OUT_SetOutputResetSrc(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUTPUTRESET_TIMCMP1);
  LL_HRTIM_OUT_SetOutputSetSrc(PIX_HRTIM, PIX_TIMOUT, LL_HRTIM_OUTPUTSET_TIMPER);


  while(!LL_HRTIM_IsActiveFlag_DLLRDY(PIX_HRTIM)) { } // wait for ready flag

  // enable the master timer
  LL_HRTIM_TIM_CounterEnable(PIX_HRTIM, LL_HRTIM_TIMER_MASTER);
}


void PIX_DMA_Setup() {
  NVIC_SetPriority(PIX_DMA_IRQN, 0);
  NVIC_EnableIRQ(PIX_DMA_IRQN);

  LL_DMA_InitTypeDef dmaInitStruct = {};
  LL_DMA_StructInit(&dmaInitStruct);

  dmaInitStruct.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  dmaInitStruct.MemoryOrM2MDstAddress = (uint32_t) &pixels_ghost;
  dmaInitStruct.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
  dmaInitStruct.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;
  dmaInitStruct.PeriphOrM2MSrcAddress = (uint32_t) PIX_CMP1AR;
  dmaInitStruct.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  dmaInitStruct.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;
  dmaInitStruct.PeriphRequest = LL_DMAMUX_REQ_HRTIM1_A;

  LL_DMA_Init(PIX_DMA, LL_DMA_CHANNEL_1, &dmaInitStruct);
}


// write timer compare values
void Ghost(Pixel pix, size_t idx) {
  for (size_t b = 0; b < 8; b++) {
    if (pix.Green & 1 << (7-b)) {
      pixels_ghost[24 * idx + b] = T1H;
    } else {
      pixels_ghost[24 * idx + b] = T0H;
    }
    if (pix.Red & 1 << (7-b)) {
      pixels_ghost[24 * idx + b + 8] = T1H;
    } else {
      pixels_ghost[24 * idx + b + 8] = T0H;
    }
    if (pix.Blue & 1 << (7-b)) {
      pixels_ghost[24 * idx + b + 16] = T1H;
    } else {
      pixels_ghost[24 * idx + b + 16] = T0H;
    }
  }
}


ErrorStatus PIX_Write(void) {
  rest_timer = 40; // 50ns plus one extra cycle of this function here
  LL_DMA_SetMemoryAddress(PIX_DMA, PIX_DMA_CH, (uint32_t) &pixels_ghost);
  LL_DMA_SetDataLength(PIX_DMA, PIX_DMA_CH, NUM_PIXELS * 24);
  LL_DMA_EnableChannel(PIX_DMA, PIX_DMA_CH);
  LL_HRTIM_TIM_SetRepetition(PIX_HRTIM, PIX_TIMER, 24 * NUM_PIXELS);
  LL_HRTIM_TIM_CounterEnable(PIX_HRTIM, PIX_TIMER);
  LL_HRTIM_EnableOutput(PIX_HRTIM, PIX_TIMOUT);
  LL_GPIO_SetOutputPin(PIX_GPIO, LL_GPIO_PIN_9);
  return SUCCESS;
}



void HRTIM_TIMA_IRQn_IRQHandler() {
  if (LL_HRTIM_IsActiveFlag_REP(PIX_HRTIM, PIX_TIMER)) {
    LL_HRTIM_DisableOutput(PIX_HRTIM, PIX_TIMOUT);
    LL_HRTIM_TIM_CounterDisable(PIX_HRTIM, PIX_TIMER);
    LL_HRTIM_ClearFlag_REP(PIX_HRTIM, PIX_TIMER);
    LL_DMA_DisableChannel(PIX_DMA, PIX_DMA_CH);
    LL_GPIO_ResetOutputPin(PIX_GPIO, LL_GPIO_PIN_9);
  }
}

/**
  ******************************************************************************
  * @file    pixel.h
  * @author  Mike Kmak
  * @brief   neopixel-related functions
  *
  *          This file contains:
  *           - nothing
  *
  ******************************************************************************
  */

#ifndef PIXEL_H
#define PIXEL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>

#include "stm32g4xx.h"
#include "stm32g4xx_ll_gpio.h"
#include "stm32g4xx_ll_hrtim.h"

// not contiguous, so how are we gonna DMA it? does that matter ?
typedef struct
{
  uint8_t Green;
  uint8_t Red;
  uint8_t Blue;
} Pixel;

// PA8 alt 13 = HRTIM_CHA1 (CN10 p23)
#define PIX_GPIO GPIOA
#define PIX_PINOUT LL_GPIO_PIN_8

#define PIX_HRTIM HRTIM1
#define PIX_TIMER LL_HRTIM_TIMER_A
#define PIX_TIMOUT LL_HRTIM_OUTPUT_TA1

#define PIX_IRQN HRTIM1_TIMA_IRQn

#define NUM_PIXELS 8 //30

ErrorStatus PIX_Init(void);
ErrorStatus PIX_Write(void);

void HRTIM_TIMA_IRQn_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif

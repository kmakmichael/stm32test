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

#define NUM_PIXELS 8 //30

ErrorStatus PIX_Init(void);
ErrorStatus PIX_Write(void);

#ifdef __cplusplus
}
#endif

#endif

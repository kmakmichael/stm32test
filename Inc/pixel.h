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
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_hrtim.h"


typedef struct {
  uint8_t Green;
  uint8_t Red;
  uint8_t Blue;
} __attribute__((packed)) Pixel;

typedef enum {
  IDLE,
  RST,
  TX
} PixelCommsState;


#define PIX_GPIO GPIOA
#define PIX_PINOUT LL_GPIO_PIN_8

#define PIX_DMA DMA1
#define PIX_DMA_CH LL_DMA_CHANNEL_1
#define PIX_DMA_IRQN DMA1_Channel1_IRQn

#define PIX_HRTIM HRTIM1
#define PIX_TIMER LL_HRTIM_TIMER_A
#define PIX_TIMOUT LL_HRTIM_OUTPUT_TA1
#define PIX_CMP1AR &HRTIM1_TIMA->CMP1xR

#define PIX_IRQN HRTIM1_TIMA_IRQn

#define NUM_PIXELS 8

#define PERIOD 1250000 / 184
#define T0H PERIOD * 400 / 1250
#define T1H PERIOD * 800 / 1250

ErrorStatus PIX_Init(void);
ErrorStatus PIX_Write(void);

void HRTIM_TIMA_IRQn_IRQHandler(void);
void DMA1_CH1_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif

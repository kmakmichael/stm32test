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

#include <stdint.h> // should i be using inttypes.h ?

// not contiguous, so how are we gonna DMA it? does that matter ?
typedef struct
{
  uint8_t Green;
  uint8_t Red;
  uint8_t Blue;
} Pixel;

#define NUM_PIXELS 1 //30

void PIX_Init(void);
void PIX_Write(void);

#ifdef __cplusplus
}
#endif

#endif

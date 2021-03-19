// DMA.h

#ifndef DMA_H
#define DMA_H

#include <stdint.h>
#include <math.h>

#include "stm32l476xx.h"

#define PI 3.14159
#define AUDIO_BUFSIZ 44100

extern volatile float frequency;
extern volatile float next_frequency;
extern volatile uint16_t last_pos; 
extern volatile uint16_t audio_buf[AUDIO_BUFSIZ];


void fill_audio_buf(float freq, uint16_t * buffer, int bufsize, uint16_t pos);

void DMA_Init(void);

void DMA2_Channel6_IRQHandler(void);


#endif /* DMA_H */

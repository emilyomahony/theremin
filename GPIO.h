// GPIO.h

#ifndef GPIO_H
#define GPIO_H

#include "stm32l476xx.h"

extern volatile int current_root;
extern volatile int major;


void EXTI_Init(void);

void EXTI2_IRQHandler(void);

#endif

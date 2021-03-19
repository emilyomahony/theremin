// PWM.h
// ultrasonic sensor setup

#ifndef PWM_H
#define PWM_H

#include "stm32l476xx.h"

extern uint32_t volatile currentValue1;
extern uint32_t volatile lastValue1;
extern uint32_t volatile overflowCount1;
extern uint32_t volatile timeInterval1;
extern uint32_t volatile fallingEdge1;

extern uint32_t volatile currentValue2;
extern uint32_t volatile lastValue2;
extern uint32_t volatile overflowCount2;
extern uint32_t volatile timeInterval2;
extern uint32_t volatile fallingEdge2;

void Input_Capture_Setup(void);

void Trigger_Setup(void);

void TIM5_IRQHandler(void);




#endif /* PWM_H */ 

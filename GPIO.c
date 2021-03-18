// GPIO.c

#include "GPIO.h"

void EXTI_Init(void) {
	// configure joystick right push to change scale
	
	// enable clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// set input mode
	GPIOA->MODER &= ~GPIO_MODER_MODE2;
	
	// set pull-down
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_1;
	
	// configure external interrupt
	SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI2;
	SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI2_PA;
	
	// configure trigger
	EXTI->RTSR1 |= EXTI_RTSR1_RT2;
	
	// enable EXTI
	EXTI->IMR1 |= EXTI_IMR1_IM2;
	
	// configure and enable in NVIC (we don't need a high priority)
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_SetPriority(EXTI2_IRQn, 1);

	
}

void EXTI2_IRQHandler(void) {
	// clear pending bit
	EXTI->PR1 |= EXTI_PR1_PIF2;
	// update scale
	if (major) {
		major = 0;
	}
	else {
		current_root++;
		major = 1;
	}
}


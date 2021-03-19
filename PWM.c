// PWM.c

#include "PWM.h"

uint32_t volatile currentValue1 = 0;
uint32_t volatile lastValue1 = 0;
uint32_t volatile overflowCount1 = 0;
uint32_t volatile fallingEdge1 = 0;

uint32_t volatile currentValue2 = 0;
uint32_t volatile lastValue2 = 0;
uint32_t volatile overflowCount2 = 0;
uint32_t volatile fallingEdge2 = 0;

// initialize data capture from sensors
void Input_Capture_Setup(void) {
	// PA0 & PA1 setup
	// enable clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	
	// set to alternate function mode
	GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1);
	GPIOA->MODER |= GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1;

	// set to no pull-up, no pull-down
	GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1);
	
	// configure timer 5 channel 1 & 2
	// enable timer
	RCC->APB1ENR1 |= RCC_APB1ENR1_TIM5EN;
	
	// select alternate function
	GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL0 | GPIO_AFRL_AFSEL1);
	GPIOA->AFR[0] |= GPIO_AFRL_AFSEL0_1 | GPIO_AFRL_AFSEL1_1;
	
	// set prescaler to 80
	TIM5->PSC |= 79UL;
	
	// enable auto-reload preload
	TIM5->CR1 |= TIM_CR1_ARPE;
	
	// set auto-reload to maximum value
	TIM5->ARR |= 0xFFFFUL;
	
	// map input capture to timer input 1 
	TIM5->CCMR1 &= ~(TIM_CCMR1_CC1S | TIM_CCMR1_CC2S); 
	TIM5->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;
	
	// capture rising & falling edges
	TIM5->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP | TIM_CCER_CC2P | TIM_CCER_CC2NP;
	
	// enable capturing
	TIM5->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E;
	
	// enable interrupt and DMA requests 
	TIM5->DIER |= TIM_DIER_CC1IE | TIM_DIER_CC2IE;
	TIM5->DIER |= TIM_DIER_CC1DE | TIM_DIER_CC2DE;
	
	// enable update interrupt
	TIM5->DIER |= TIM_DIER_UIE;
	
	// enable update generation
	TIM5->EGR |= TIM_EGR_UG;
	
	// clear update interrupt 
	TIM5->SR &= ~TIM_SR_UIF;
	
	// set counter to upcounting
	TIM5->CR1 &= ~TIM_CR1_DIR;
	
	// enable counter
	TIM5->CR1 |= TIM_CR1_CEN;
	
	// NVIC timer 5
	NVIC_EnableIRQ(TIM5_IRQn);
	NVIC_SetPriority(TIM5_IRQn, 2);
}

// track overflows, update volume / frequency based on sensor input
void TIM5_IRQHandler(void) {
	if (TIM5->SR & TIM_SR_UIF) {
		// an overflow has occurred
		overflowCount1++;
		overflowCount2++;
		// clear interrupt flag
		TIM5->SR &= ~TIM_SR_UIF;
	}
	if (TIM5->SR & TIM_SR_CC1IF) {
		// update values
		lastValue1 = currentValue1; 
		currentValue1 = TIM5->CCR1;
		// if this is the falling edge
		if (fallingEdge1){
			timeInterval1 = currentValue1  + (overflowCount1 * 0xFFFFUL) - lastValue1;
		}
		// zero overflow count on any edge
		overflowCount1 = 0;
		// track which edge we're seeing
		fallingEdge1 = ~fallingEdge1;
		// clear interrupt flag
		TIM5->SR &= ~TIM_SR_CC1IF;
	}	
	if (TIM5->SR & TIM_SR_CC2IF) {
		// update values
		lastValue2 = currentValue2; 
		currentValue2 = TIM5->CCR2;
		// if this is the falling edge
		if (fallingEdge2){
			timeInterval2 = currentValue2  + (overflowCount2 * 0xFFFFUL) - lastValue2;
		}
		// zero overflow count on any edge
		overflowCount2 = 0;
		// track which edge we're seeing
		fallingEdge2 = ~fallingEdge2;
		// clear interrupt flag
		TIM5->SR &= ~TIM_SR_CC2IF;
	}	
}

// initialize periodic trigger sensor pulse to receive data
void Trigger_Setup(void) {
	// PE11 setup
	// enable clock
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN; 
	
	// set to alternate function mode
	GPIOE->MODER &= ~GPIO_MODER_MODE11;
	GPIOE->MODER |= GPIO_MODER_MODE11_1;
	
	// select alternate function
	GPIOE->AFR[1] &= ~GPIO_AFRH_AFSEL11;
	GPIOE->AFR[1] |= GPIO_AFRH_AFSEL11_0;
	
	// no pull-up, no pull-down
	GPIOE->PUPDR &= ~GPIO_PUPDR_PUPD11;
	
	// set output type push-pull
	GPIOE->OTYPER &= ~GPIO_OTYPER_OT11;
	
	// set very high output speed
	GPIOE->OSPEEDR |= GPIO_OSPEEDR_OSPEED11; 
	
	// configure timer 1 channel 2
	// enable timer 
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; 
	
	// set prescaler to 80
	TIM1->PSC |= 79UL;
	
	// enable auto-reload preload
	TIM1->CR1 |= TIM_CR1_ARPE;
	
	// set auto-reload to max value (2^16 - 1)
	TIM1->ARR |= 0xFFFFUL;
	
	// set CCR value 
	// 80MHz / 80 = 1MHz 
	// we want at least a 10us pulse
	// each tick = 1us
	// 15us to be safe
	TIM1->CCR2 = 15UL;
	
	// set output control mode to PWM mode 1
	// enable output compare preload
	TIM1->CCMR1 &= ~TIM_CCMR1_OC2M;
	TIM1->CCMR1 |= (TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE);
	
	// set output polarity high
	TIM1->CCER &= ~TIM_CCER_CC2P;
	
	// enable channel 2 output 
	TIM1->CCER |= TIM_CCER_CC2E;
	
	// set off-state selection for run mode
	TIM1->BDTR |= TIM_BDTR_OSSR;
	
	// enable main output BDTR
	TIM1->BDTR |= TIM_BDTR_MOE;

	
	// enable update generation
	TIM1->EGR |= TIM_EGR_UG;
	
	// enable & clear update interrupt
	TIM1->DIER |= TIM_DIER_UIE;
	TIM1->SR &= ~TIM_SR_UIF;
	
	// set counter to upcounting
	TIM1->CR1 &= ~TIM_CR1_DIR;
	
	// enable counter
	TIM1->CR1 |= TIM_CR1_CEN;
}

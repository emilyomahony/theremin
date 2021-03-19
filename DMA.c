// DMA.c

#include "DMA.h"

volatile uint16_t audio_buf[AUDIO_BUFSIZ];
volatile uint16_t last_pos = 0;

// fills audio buffer that we feed into I2S FIFO
void fill_audio_buf(float freq, uint16_t * buffer, int bufsize, uint16_t pos) {
	float coef = (float)PI * freq / (float)AUDIO_BUFSIZ;
	for (int i = 0; i < bufsize; i++) {		
		buffer[i] = (uint16_t)((float)16382.0 * sinf(( coef * (float)(i) )) + (float)16384.0);
	}
}

// initializes transfer of sound data from audio buffer to SAI1 Block A (I2S) FIFO
void DMA_Init(void) {

	// initialize audio buffer with startup tone
	fill_audio_buf(220.0, (uint16_t*)audio_buf, AUDIO_BUFSIZ, 0);

	// enable clock
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;

	// disable DMA
	DMA2_Channel6->CCR &= ~DMA_CCR_EN;

	// peripheral data = 16 bits
	DMA2_Channel6->CCR &=~DMA_CCR_PSIZE;
	DMA2_Channel6->CCR |= DMA_CCR_PSIZE_0;

	// memory data = 16 bits
	DMA2_Channel6->CCR &=~DMA_CCR_MSIZE;
	DMA2_Channel6->CCR |= DMA_CCR_MSIZE_0;

	// disable peripheral increment mode
	DMA2_Channel6->CCR &= ~DMA_CCR_PINC;

	// enable memory increment mode
	DMA2_Channel6->CCR |= DMA_CCR_MINC;

	// data goes from memory to peripheral
	DMA2_Channel6->CCR |= DMA_CCR_DIR;

	// circular buffer
	DMA2_Channel6->CCR |= DMA_CCR_CIRC;

	// amount of data to transfer
	DMA2_Channel6->CNDTR = AUDIO_BUFSIZ;

	// peripheral address
	DMA2_Channel6->CPAR = (uint32_t)&(SAI1_Block_A->DR);

	// memory address
	DMA2_Channel6->CMAR = (uint32_t)&(audio_buf);

	// channel select
	DMA2_CSELR->CSELR &= ~DMA_CSELR_C6S;
	// SAI1_Block_A
	DMA2_CSELR->CSELR |= 1UL << 20;

	// half transfer interrupt
	DMA2_Channel6->CCR |= DMA_CCR_HTIE;
	// complete transfer interrupt
	DMA2_Channel6->CCR |= DMA_CCR_TCIE;

	// set highest priority
	NVIC_SetPriority(DMA2_Channel6_IRQn, 0);

	// enable interrupt
	NVIC_EnableIRQ(DMA2_Channel6_IRQn);
	
	// enable DMA
	DMA2_Channel6->CCR |= DMA_CCR_EN;
	return;
}

// half & complete transfer reload for audio buffer 
void DMA2_Channel6_IRQHandler(void) {
	// reload after getting through half of the buffer
	uint16_t temp_pos;
	next_frequency = frequency;
	if (DMA2->ISR & DMA_ISR_HTIF6) {
		fill_audio_buf(next_frequency, (uint16_t*)audio_buf, AUDIO_BUFSIZ / 2, last_pos);
	}
	else if (DMA2->ISR & DMA_ISR_TCIF6) {
		// temp_pos = fill_audio_buf(next_frequency, audio_buf + (AUDIO_BUFSIZ / 2), AUDIO_BUFSIZ / 2, last_pos);
		fill_audio_buf(next_frequency, (uint16_t*)audio_buf + (AUDIO_BUFSIZ / 2), AUDIO_BUFSIZ / 2, last_pos);
	}
	last_pos = temp_pos;
	
	// clear interrupt flag
	DMA2->IFCR |= DMA_IFCR_CGIF6;	
}
	


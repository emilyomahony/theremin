#include "stm32l476xx.h"

#include <stdio.h>
#include <math.h>
#include <time.h>

#include "I2C.h"
#include "LCD.h"
#include "SysClock.h"
#include "codec.h"
#include "PWM.h"
#include "GPIO.h"

#define FREQ	44100

#define AUDIO_BUFSIZ FREQ
	
#define PI 3.14159
	
static uint16_t audio_buf[AUDIO_BUFSIZ];

volatile float frequency;
volatile float next_frequency;
volatile uint16_t last_pos;
volatile int increasing;

uint32_t volatile currentValue1 = 0;
uint32_t volatile lastValue1 = 0;
uint32_t volatile overflowCount1 = 0;
uint32_t volatile timeInterval1 = 0;
uint32_t volatile fallingEdge1 = 0;

uint32_t volatile currentValue2 = 0;
uint32_t volatile lastValue2 = 0;
uint32_t volatile overflowCount2 = 0;
uint32_t volatile timeInterval2 = 0;
uint32_t volatile fallingEdge2 = 0;

volatile int current_root;
volatile int major;

void fill_audio_buf(float freq, uint16_t * buffer, int bufsize, uint16_t pos);


// DMA functions

void DMA_Init(void) {

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

void DMA2_Channel6_IRQHandler(void) {
	// reload after getting through half of the buffer
	uint16_t temp_pos;
	next_frequency = frequency;
	if (DMA2->ISR & DMA_ISR_HTIF6) {
		fill_audio_buf(next_frequency, audio_buf, AUDIO_BUFSIZ / 2, last_pos);
	}
	else if (DMA2->ISR & DMA_ISR_TCIF6) {
		// temp_pos = fill_audio_buf(next_frequency, audio_buf + (AUDIO_BUFSIZ / 2), AUDIO_BUFSIZ / 2, last_pos);
		fill_audio_buf(next_frequency, audio_buf + (AUDIO_BUFSIZ / 2), AUDIO_BUFSIZ / 2, last_pos);
	}
	last_pos = temp_pos;
	
	// clear interrupt flag
	DMA2->IFCR |= DMA_IFCR_CGIF6;	
}
	
// end DMA functions


// sound functions

void fill_audio_buf(float freq, uint16_t * buffer, int bufsize, uint16_t pos) {
	float coef = (float)PI * freq / (float)FREQ;
	for (int i = 0; i < bufsize; i++) {		
		buffer[i] = (uint16_t)((float)16382.0 * sinf(( coef * (float)(i) )) + (float)16384.0);
	}
}

float round4(float n) {
	if ((int)n % 4 == 0 || (int)n % 4 == 1) 
		return (int) n - (int) n % 4;
	else 
		return (int) n + 4 - (int) n % 4;
}

float get_freq(float distance) {
	float freq = (float)880.0 * powf((float)2.0,( (distance - ((int) distance % 5) ) / 5) / 12);
	if ((int)freq % 2 == 0) {
		return (int)freq;
	}
	else {
		return (int)freq + 1;
	}
}

void get_scale(float * frequencies, int root, int major) {
	// put the 8 tones of the scale into frequencies starting at the first note from A5
	// root is the number of tones from A5
	int major_steps [] = {0, 2, 4, 5, 7, 9, 11, 12};
	int minor_steps [] = {0, 2, 3, 5, 7, 8, 10, 12};
	int * steps = major ? major_steps : minor_steps; 
	float first = round4((float)880.0 * powf((float)2.0, (float)(root) / (float)12.0));
	for (int i = 0; i < 8; i++) {
		frequencies[i] = round4(first * powf((float)2.0, (float)steps[i] / (float)12.0));
	}
}



float get_scale_freq(float distance, int root, int major) {

	float frequencies [8];
	get_scale(frequencies, root, major);

	if (distance - 5 < 5) {
		return frequencies[0];
	}
	else if (distance - 5 > 30) {
		return frequencies[6];
	}
	else {
		return frequencies[(int)(distance - 5 - ((int)distance % 5)) / 5];
	}
}

int get_vol(float distance) {
	if (distance > 15) {
		return 60;
	}
	else if (distance < 5) {
		return 10;
	}
	else {
		return ((int)distance + 40);
	}
}

// end sound functions


void display_root(int root) {
	char message [8];
	if (major) {
		switch (root) {
			case 0:
				sprintf(message, " A  M ");
			break;
			case 1:
				sprintf(message, " Bb M ");
			break;
			case 2: 
				sprintf(message, " B  M ");
			break;
			case 3: 
				sprintf(message, " C  M ");
			break;
			case 4: 
				sprintf(message, " Db M ");
			break;
			case 5: 
				sprintf(message, " D  M ");
			break;
			case 6: 
				sprintf(message, " Eb M ");
			break;
			case 7: 
				sprintf(message, " E  M ");
			break;
			case 8: 
				sprintf(message, " F  M ");
			break;
			case 9: 
				sprintf(message, " Gb M ");
			break;
			case 10: 
				sprintf(message, " G  M ");
			break;
			case 11: 
				sprintf(message, " Ab M ");
			break;
		}
	}
	else {
		switch (root) {
			case 0:
				sprintf(message, " A  m ");
			break;
			case 1:
				sprintf(message, " Bb m ");
			break;
			case 2: 
				sprintf(message, " B  m ");
			break;
			case 3: 
				sprintf(message, " C  m ");
			break;
			case 4: 
				sprintf(message, " Db m ");
			break;
			case 5: 
				sprintf(message, " D  m ");
			break;
			case 6: 
				sprintf(message, " Eb m ");
			break;
			case 7: 
				sprintf(message, " E  m ");
			break;
			case 8: 
				sprintf(message, " F  m ");
			break;
			case 9: 
				sprintf(message, " Gb m ");
			break;
			case 10: 
				sprintf(message, " G  m ");
			break;
			case 11: 
				sprintf(message, " Ab m ");
			break;
		}
	}
	LCD_DisplayString((uint8_t*)message);
}

int main(void) {
	
	// System Clock = 80 MHz
	System_Clock_Init(); 
	
	// Initialize LCD
	LCD_Initialization();
	LCD_Clear();
	
	// Initialize I2C
	I2C_GPIO_Init();
	I2C_Initialization();
	
	// Initialize ultrasonic sensor PWM
	Input_Capture_Setup();
	Trigger_Setup();
	
	// Initialize CS43L22 codec
	codec_init();
	codec_play();
		
	// startup tone
	frequency = 220.0;

	// fill buffer to start with
	fill_audio_buf(frequency, audio_buf, AUDIO_BUFSIZ, 0);
	
	// Initialize DMA transfer
	DMA_Init();
	
	// initialize button
	EXTI_Init();
	
	// initial volume = max
	int volume = 60;
	int last_volume = 60;
	int i;
	
	uint32_t temp1 = 0;
	uint32_t temp2 = 0;
	
	// start with A major
	current_root = 0;
	major = 1;

codec_volume(40);
	while(1) {	
		
		// get distance from sensors
		if (timeInterval1/58.0 < 100 ) {
			temp1 = timeInterval1 / 58.0;
		}
		if (timeInterval2/58.0 < 50) {
			temp2 = timeInterval2 / 58.0;
		}
		
		// update frequency
		frequency = get_scale_freq(temp1, current_root, major);
		display_root(current_root);
		
		// update volume
		volume = get_vol(temp2);
		// increase / decrease volume smoothly
		while (volume > last_volume) {
			last_volume++;
			codec_volume(last_volume);
			for (i = 0; i < 400000; i++);
		}
		while (volume < last_volume) {
			last_volume--;
			codec_volume(last_volume);
			for (i = 0; i < 400000; i++);
		}
	}
}

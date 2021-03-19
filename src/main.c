#include "stm32l476xx.h"

#include <stdio.h>
#include <math.h>

#include "I2C.h"
#include "LCD.h"
#include "SysClock.h"
#include "codec.h"
#include "PWM.h"
#include "GPIO.h"
#include "DMA.h"
#include "music.h"

volatile float frequency = 0;
volatile float next_frequency = 0;

uint32_t volatile timeInterval1 = 0;
uint32_t volatile timeInterval2 = 0;

volatile int current_root;
volatile int major;

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
		display_root(current_root, major);
		
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

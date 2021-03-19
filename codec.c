// codec.c

#include "codec.h"
#include "LCD.h"
#include <stdio.h>

// helpful for checking if we're communicating with the sound chip
void codec_id(int msg) {
	uint8_t send = CS43L22_REG_ID;
	I2C_SendData(I2C1, ADDR, &send, 1);
	uint8_t receive;
	I2C_ReceiveData(I2C1, ADDR, &receive, 1);
	uint8_t message [8];
	if ((receive << 3) == 0x1c) {
		sprintf((char*)message, "yes %d", msg);
	}
	else {
		sprintf((char*)message, "no %d", msg);
	}
	LCD_DisplayString(message);
	for (int d = 0; d < 500000; d++);
}

// CS43L22 startup sequence (from datasheet)
void codec_startup(void) {
	// init GPIO reset pin PE3
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	
	// set GP output mode
	GPIOE->MODER &= ~GPIO_MODER_MODE3;
	GPIOE->MODER |= GPIO_MODER_MODE3_0;
	
	// hold !reset low until power stabilizes
	GPIOE->ODR &= ~GPIO_ODR_OD3;
	for (int delay = 0; delay < 40000; delay++);
	
	// bring !reset high
	GPIOE->ODR |= GPIO_ODR_OD3;
	
	// load initialization sequence
	uint8_t data[2];
	
	data[0] = 0x0; 
	data[1] = 0x99;
	I2C_SendData(I2C1, ADDR, data, 2);
	data[0] = 0x47; 
	data[1] = 0x80;
	I2C_SendData(I2C1, ADDR, data, 2);
	data[0] = 0x32; 
	data[1] = 0xbb;
	I2C_SendData(I2C1, ADDR, data, 2);
	data[0] = 0x32; 
	data[1] = 0x3b;
	I2C_SendData(I2C1, ADDR, data, 2);
	data[0] = 0x0; 
	data[1] = 0x0;
	I2C_SendData(I2C1, ADDR, data, 2);
}

// set up GPIO pins for SAI function
void SAI_GPIO_init(void) {
	// configure SAI GPIO
	// PE2 = MCK, PE5 = SCK, PE6 = SD, PE4 = FS
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIOEEN;
	
	// alternate function mode
	GPIOE->MODER &= ~(
		GPIO_MODER_MODE2 | 
		GPIO_MODER_MODE5 | 
		GPIO_MODER_MODE6 | 
		GPIO_MODER_MODE4
	);
	GPIOE->MODER |= (
		GPIO_MODER_MODE2_1 | 
		GPIO_MODER_MODE5_1 | 
		GPIO_MODER_MODE6_1 | 
		GPIO_MODER_MODE4_1
	);
	
	// select SAI alternate functions (AF13)
	GPIOE->AFR[0] |= (
		GPIO_AFRL_AFSEL2 | 
		GPIO_AFRL_AFSEL5 | 
		GPIO_AFRL_AFSEL6 | 
		GPIO_AFRL_AFSEL4
	);
	GPIOE->AFR[0] &= ~(
		GPIO_AFRL_AFSEL2_1 | 
		GPIO_AFRL_AFSEL5_1 | 
		GPIO_AFRL_AFSEL6_1 | 
		GPIO_AFRL_AFSEL4_1
	);
	
	// no pull-up, no pull-down
	GPIOE->PUPDR &= ~(
		GPIO_PUPDR_PUPD2 | 
		GPIO_PUPDR_PUPD5 | 
		GPIO_PUPDR_PUPD6 | 
		GPIO_PUPDR_PUPD4
	);
	
	// push-pull output type
	GPIOE->OTYPER &= ~(
		GPIO_OTYPER_OT2 | 
		GPIO_OTYPER_OT5 | 
		GPIO_OTYPER_OT6 | 
		GPIO_OTYPER_OT4
	);
	
	// very high output speed
	GPIOE->OSPEEDR |= (
		GPIO_OSPEEDR_OSPEED2 | 
		GPIO_OSPEEDR_OSPEED5 | 
		GPIO_OSPEEDR_OSPEED6 | 
		GPIO_OSPEEDR_OSPEED4
	);
	
}

// initialize SAI for I2S
void SAI_init(void) {
	// enable clock
	RCC->APB2ENR |= RCC_APB2ENR_SAI1EN;
	
	// disable SAI
	SAI1_Block_A->CR1 &= ~SAI_xCR1_SAIEN;
	// wait for disable
	while ((SAI1_Block_A->CR1 & SAI_xCR1_SAIEN) != 0); 
	
	// set "Free" protocol for I2S later
	SAI1_Block_A->CR1 &= ~SAI_xCR1_PRTCFG;
	
	// set master transmission
	SAI1_Block_A->CR1 &= ~SAI_xCR1_MODE;
	
	// first bit MSB
	SAI1_Block_A->CR1 &= ~SAI_xCR1_LSBFIRST;
	
	// generate on SCK falling, sample on SCK rising
	SAI1_Block_A->CR1 |= SAI_xCR1_CKSTR;	
	
	// 16 bit samples
	SAI1_Block_A->CR1 &= ~SAI_xCR1_DS;
	SAI1_Block_A->CR1 |= SAI_xCR1_DS_2;
	
	// don't divide master clock
	SAI1_Block_A->CR1 &= ~SAI_xCR1_MCKDIV;

	// set fifo threshold to 1/4 full
	SAI1_Block_A->CR2 &= ~SAI_xCR2_FTH;
	SAI1_Block_A->CR2 |= SAI_xCR2_FTH_0;
	
	// frame length = 32
	SAI1_Block_A->FRCR &= ~SAI_xFRCR_FRL;
	SAI1_Block_A->FRCR |= (32 - 1);
	
	// frame offset before first bit
	SAI1_Block_A->FRCR |= SAI_xFRCR_FSOFF;
	
	// FS channel id
	SAI1_Block_A->FRCR |= SAI_xFRCR_FSDEF;
	
	// frame polarity active low
	SAI1_Block_A->FRCR &= ~SAI_xFRCR_FSPOL;
	
	// active frame length = 16
	SAI1_Block_A->FRCR &= ~SAI_xFRCR_FSALL;
	SAI1_Block_A->FRCR |= SAI_xFRCR_FSALL_0 | SAI_xFRCR_FSALL_1 | SAI_xFRCR_FSALL_2 | SAI_xFRCR_FSALL_3;
	
	// slot first bit offset = 0
	SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_FBOFF;
	
	// slot size = 16 bits
	SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_SLOTSZ;
	SAI1_Block_A->SLOTR |= SAI_xSLOTR_SLOTSZ_0;
	
	// slot active = first two
	SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_SLOTEN;
	SAI1_Block_A->SLOTR |= 0x3 << 16;
	
	// slot number = 2 (stereo)
	SAI1_Block_A->SLOTR &= ~SAI_xSLOTR_NBSLOT;
	SAI1_Block_A->SLOTR |= SAI_xSLOTR_NBSLOT_0;
	
	// enable DMA requests
	SAI1_Block_A->CR1 |= SAI_xCR1_DMAEN;
	
	// enable SAI
	SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;
}

// configure sound chip 
void codec_init(void) {
	// boot code
	codec_startup();
	SAI_GPIO_init();
	SAI_init();
	
	uint8_t data [2];
	
	// auto-detect clock
	data[0] = CS43L22_REG_CLOCKING_CTL;
	data[1] = 0x80;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// interface control
	// slave mode, not inverted, DSP disabled, I2S, 16 bit
	data[0] = CS43L22_REG_INTERFACE_CTL1;
	data[1] = 0x07;
	I2C_SendData(I2C1, ADDR, data, 2);

	// set volume
	codec_volume(60);
	
	// disable analog soft ramp
	data[0] = CS43L22_REG_ANALOG_ZC_SR_SETT;
	data[1] = 0x0;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// disable digital soft ramp
	data[0] = CS43L22_REG_MISC_CTL;
	data[1] = 0x04;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// disable limiter attack level
	data[0] = CS43L22_REG_LIMIT_CTL1;
	data[1] = 0x0;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// adjust bass & treble levels
	data[0] = CS43L22_REG_TONE_CTL;
	data[1] = 0x0f;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// adjust PCM volume level
	data[0] = CS43L22_REG_PCMA_VOL;
	data[1] = 0x0a;
	I2C_SendData(I2C1, ADDR, data, 2);
	data[0] = CS43L22_REG_PCMB_VOL;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// make sure beep is off
	data[0] = CS43L22_REG_BEEP_TONE_CFG;
	data[1] = 0x0;
	I2C_SendData(I2C1, ADDR, data, 2);
}

// enable sound chip 
void codec_play(void) {
	uint8_t data [2];
	// enable digital soft ramp
	data[0] = CS43L22_REG_MISC_CTL;
	data[1] = 0x06;
	I2C_SendData(I2C1, ADDR, data, 2);

	// enable output device
	data[0] = CS43L22_REG_POWER_CTL2;
	data[1] = 0xaf;
	I2C_SendData(I2C1, ADDR, data, 2);
	
	// power on codec
	data[0] = CS43L22_REG_POWER_CTL1;
	data[1] = 0x9e;
	I2C_SendData(I2C1, ADDR, data, 2);

}

// change volume
void codec_volume(int volume){
	if (volume > 60) volume = 60;
	uint8_t convertedvol = (((volume) > 100)? 100:((uint8_t)(((volume) * 255) / 100)));
	
	uint8_t data [2];
	
	if (volume > 0xe6) {
		data[0]=CS43L22_REG_MASTER_A_VOL;
		data[1]=convertedvol-0xe7;
		I2C_SendData(I2C1,ADDR,data,2);

		data[0]=CS43L22_REG_MASTER_B_VOL;	
		data[1]=convertedvol-0xe7;
		I2C_SendData(I2C1,ADDR,data,2);
	} 
	else {
		data[0]=CS43L22_REG_MASTER_A_VOL;
		data[1]=convertedvol+0x19;
		I2C_SendData(I2C1,ADDR,data,2);

		data[0]=CS43L22_REG_MASTER_B_VOL;
		data[1]=convertedvol+0x19;
		I2C_SendData(I2C1,ADDR,data,2);
	}

	// be sure headphone is unmuted 
	data[0]=CS43L22_REG_HEADPHONE_A_VOL;	
	data[1]=0x00;
	I2C_SendData(I2C1,ADDR,data,2);

	data[0]=CS43L22_REG_HEADPHONE_B_VOL;		
	data[1]=0x00;
	I2C_SendData(I2C1,ADDR,data,2);
}

// function for manually sending I2S data to sound chip
// not necessary since we use DMA to transmit data 
int codec_send(uint16_t *audio_left, uint16_t *audio_right, uint16_t buf_size) {

	uint32_t count=buf_size;
	uint32_t temp;
	uint16_t *left_data=audio_left;
	uint16_t *right_data=audio_right;
	int left=1;

	if ((audio_left==0) || (audio_right==0) || (buf_size==0)) return 0;

	// if not enabled, fill FIFO and enable 
	if ((SAI1_Block_A->CR1 & SAI_xCR1_SAIEN)==0) {
		while (((SAI1_Block_A->SR & SAI_xSR_FLVL) != (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2))
			&& (count > 0U)) {

			if (left) {
				temp = (uint32_t)(*left_data);
				left_data++;
				left=0;
			}
			else {
				temp = ((uint32_t)(*right_data));
				right_data++;
				left=1;
			}
			SAI1_Block_A->DR |= temp;

			count--;
		}
		SAI1_Block_A->CR1 |= SAI_xCR1_SAIEN;
	}
	while (count > 0U) {
		// write data if the FIFO is not full 
		if ((SAI1_Block_A->SR & SAI_xSR_FLVL) != (SAI_xSR_FLVL_0 | SAI_xSR_FLVL_2)) {
			if (left) {
				temp = (uint32_t)(*left_data);
				left_data++;
				left=0;
			}
			else {
				temp = (uint32_t)(*right_data);
				right_data++;
				left=1;
			}
			SAI1_Block_A->DR |= temp;
			count--;
		}
	}
	return 0;
}

// music.c

#include "music.h"
#include "LCD.h"

#include <stdio.h>

// utility function for rounding to nearest 4Hz frequency
float round4(float n) {
	if ((int)n % 4 == 0 || (int)n % 4 == 1) 
		return (int) n - (int) n % 4;
	else 
		return (int) n + 4 - (int) n % 4;
}

// convert distance from sensors to frequency (we don't use this at the moment)
float get_freq(float distance) {
	float freq = (float)880.0 * powf((float)2.0,( (distance - ((int) distance % 5) ) / 5) / 12);
	if ((int)freq % 2 == 0) {
		return (int)freq;
	}
	else {
		return (int)freq + 1;
	}
}

// get all the frequencies from a scale 
void get_scale(float * frequencies, int root, int major) {
	int major_steps [] = {0, 2, 4, 5, 7, 9, 11, 12};
	int minor_steps [] = {0, 2, 3, 5, 7, 8, 10, 12};
	int * steps = major ? major_steps : minor_steps; 
	float first = round4((float)880.0 * powf((float)2.0, (float)(root) / (float)12.0));
	for (int i = 0; i < 8; i++) {
		frequencies[i] = round4(first * powf((float)2.0, (float)steps[i] / (float)12.0));
	}
}

// get the frequency from a scale based on sensor distance
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

// convert distance to volume
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

// display the scale we're using currently
void display_root(int root, int major) {
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



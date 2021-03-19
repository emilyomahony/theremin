// music.h

#ifndef MUSIC_H
#define MUSIC_H

#include <stdint.h>
#include <math.h>

#include "stm32l476xx.h"

float get_freq(float distance);
void get_scale(float * frequencies, int root, int major);
float get_scale_freq(float distance, int root, int major);
	
int get_vol(float distance);

void display_root(int root, int major);

float round4(float n);
	
#endif /* MUSIC_H */

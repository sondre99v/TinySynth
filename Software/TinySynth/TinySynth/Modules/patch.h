/*
 * patch.h
 *
 * Created: 26/04/19 16:08:40
 *  Author: Sondre
 */


#ifndef PATCH_H_
#define PATCH_H_

#include "oscillator.h"

typedef struct {
	uint8_t oscA_octave;
	waveform_t oscA_wave;
	uint8_t oscB_octave;
	waveform_t oscB_wave;
	uint8_t oscB_detune;
	bool oscB_enabled;
	bool sync;
	uint8_t eg_rise_speed;
	uint8_t eg_fall_speed;
} patch_t;

void patch_init(void);

void patch_cycle_oscA_octave(void);
void patch_cycle_oscA_wave(void);
void patch_cycle_oscB_octave(void);
void patch_cycle_oscB_wave(void);
void patch_toggle_eg_rise(void);
void patch_toggle_eg_fall(void);
void patch_cycle_oscB_detune(void);
void patch_toggle_sync(void);


#endif /* PATCH_H_ */
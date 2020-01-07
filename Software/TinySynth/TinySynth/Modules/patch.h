/*
 * patch.h
 *
 * Created: 26/04/19 16:08:40
 *  Author: Sondre
 */


#ifndef PATCH_H_
#define PATCH_H_

#include "oscillator.h"

typedef enum {
	EFFECT_NONE = 0,
	EFFECT_FILTER = 1,
	EFFECT_HIT = 2,
	EFFECT_SHAKE =3
} effect_t;

typedef struct {
	uint8_t oscA_note_offset;
	waveform_t oscA_wave;
	uint8_t oscB_note_offset;
	waveform_t oscB_wave;
	bool oscA_enabled;
	bool oscB_enabled;
	bool slide;
	uint8_t eg_rise_speed;
	uint8_t eg_fall_speed;
	effect_t effect;
} patch_t;

void patch_init(void);

void patch_cycle_oscA_pitch(void);
void patch_cycle_oscA_wave(void);
void patch_cycle_oscB_pitch(void);
void patch_cycle_oscB_wave(void);
void patch_toggle_eg_rise(void);
void patch_toggle_eg_fall(void);
void patch_toggle_slide(void);
void patch_toggle_effect(void);


#endif /* PATCH_H_ */
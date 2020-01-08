/*
 * lfo.c
 *
 * Created: 07-01-2020 23:05:40
 *  Author: Sondre
 */ 

#include "lfo.h"
#include <stdlib.h>

#define SPEED_MULTIPLIER 32

static lfo_t LFO_DATA_1 = {0};

lfo_t* const LFO_1 = &LFO_DATA_1;


static int8_t wave_sqr(uint16_t angle) {
	return (angle & 0x8000) ? 127 : -128;
}

static int8_t wave_tri(uint16_t angle) {
	int stage = angle >> 14;
	int stage_angle = (angle & 0x3FFF) >> 7;
	switch (stage) {
		case 0: return stage_angle;
		case 1: return 127 - stage_angle;
		case 2: return -stage_angle;
		case 3: return -128 + stage_angle;
		default: return 0;
	}
}

static int8_t wave_sin(uint16_t angle) {
	const int8_t lut[] = {
		0, 48, 89, 117, 127, 117, 89, 48, 0, -49, -90, -118, -127, -118, -90, -49
	};

	uint8_t i1 = angle >> 12;
	int8_t v1 = lut[i1];
	int8_t v2 = lut[(i1 + 1) % 16];
	uint8_t x = (angle & 0xFFF) >> 4;

	return v1 + (((int16_t)(v2 - v1) * x) >> 8);
}


void lfo_init(lfo_t* lfo)
{
	lfo->waveshape = LFO_WAVE_SIN;
	lfo->current_phase = 0;
	lfo->speed = 60;
	lfo->value = 0;
}

void lfo_update(lfo_t* lfo)
{
	lfo->current_phase += lfo->speed * SPEED_MULTIPLIER;

	switch(lfo->waveshape) {
		case LFO_WAVE_SQUARE:   lfo->value = wave_sqr(lfo->current_phase); break;
		case LFO_WAVE_TRIANGLE: lfo->value = wave_tri(lfo->current_phase); break;
		case LFO_WAVE_SIN:      lfo->value = wave_sin(lfo->current_phase); break;
	}
}

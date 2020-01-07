/*
 * lfo.h
 *
 * Created: 07-01-2020 23:05:29
 *  Author: Sondre
 */ 


#ifndef LFO_H_
#define LFO_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum {
	LFO_WAVE_SQUARE,
	LFO_WAVE_TRIANGLE,
	LFO_WAVE_SIN
} lfo_wave_t;

typedef struct {
	lfo_wave_t waveshape;
	uint8_t speed;
	uint16_t current_phase;
	int8_t value;
} lfo_t;


extern lfo_t* const LFO_1;


void lfo_init(lfo_t* lfo);
void lfo_update(lfo_t* lfo);


#endif /* LFO_H_ */
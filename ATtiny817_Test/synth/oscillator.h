/*
 * oscillator.h
 *
 * Created: 2019-02-19 21:57:40
 *  Author: Sondre
 */ 


#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_


#include <stdint.h>


typedef enum {
	WAVE_NONE,
	WAVE_SAW,
	WAVE_SQUARE,
	WAVE_TRIANGLE,
	WAVE_SINE
} waveform_t;


typedef struct {
	uint16_t frequency_dHz;
	uint8_t amplitude;
	waveform_t waveform;
	uint8_t filter_value;
} oscillator_t;


extern volatile oscillator_t OscillatorA;
extern volatile oscillator_t OscillatorB;


void oscillator_start(oscillator_t* osc);
void oscillator_stop(oscillator_t* osc);

#endif /* OSCILLATOR_H_ */
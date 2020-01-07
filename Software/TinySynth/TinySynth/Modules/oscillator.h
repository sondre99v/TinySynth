/*
 * oscillator.h
 *
 * Created: 19/04/19 12:17:44
 *  Author: Sondre
 */


#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_


#include <stdint.h>
#include <stdbool.h>


typedef enum {
	WAVE_SILENCE = -1,
	WAVE_SINE = 0,
	WAVE_TRIANGLE = 1,
	WAVE_SQUARE = 2,
	WAVE_SAW = 3,
	WAVE_NOISE = 4
} waveform_t;

typedef enum {
	OSCILLATOR_A = 0,
	OSCILLATOR_B = 1
} oscillator_t;


void oscillator_init(void);

void oscillator_update(oscillator_t oscillator);
void oscillator_set_sources(oscillator_t oscillator, uint8_t* note_input, int8_t* bend_input, uint8_t* amplitude_input);
void oscillator_set_waveform(oscillator_t oscillator, waveform_t waveform);
void oscillator_set_octave(oscillator_t oscillator, uint8_t octave);
void oscillator_set_sync(bool enabled);
void oscillator_set_sweep_speed(oscillator_t oscillator, uint16_t sweep_speed);

#endif /* OSCILLATOR_H_ */
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

void oscillator_set_waveform(oscillator_t oscillator, waveform_t waveform);
void oscillator_set_frequency(oscillator_t oscillator, uint16_t frequency_dHz);
void oscillator_set_amplitude(oscillator_t oscillator, uint8_t amplitude);
void oscillator_set_octave(oscillator_t oscillator, uint8_t octave);
void oscillator_set_detune(oscillator_t oscillator, uint8_t detune);
void oscillator_set_sync(bool enabled);

#endif /* OSCILLATOR_H_ */
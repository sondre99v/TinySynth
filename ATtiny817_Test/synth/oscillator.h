/*
 * oscillator.h
 *
 * Created: 2019-02-19 21:57:40
 *  Author: Sondre
 */ 


#ifndef OSCILLATOR_H_
#define OSCILLATOR_H_


#include <stdint.h>
#include <stdbool.h>


typedef enum {
	WAVE_NONE,
	WAVE_SAW,
	WAVE_SQUARE,
	WAVE_TRIANGLE,
	WAVE_SINE
} waveform_t;

typedef enum {
	OSCILLATOR_NONE,
	OSCILLATOR_A,
	OSCILLATOR_B
} oscillator_t;


void osc_init();

void osc_set_waveform(oscillator_t osc, waveform_t waveform);
void osc_set_frequency(oscillator_t osc, uint16_t frequency_dHz);
void osc_set_amplitude(oscillator_t osc, uint8_t amplitude);
void osc_set_filter_value(oscillator_t osc, uint8_t filter_value);
void osc_set_sync(bool enabled);

#endif /* OSCILLATOR_H_ */
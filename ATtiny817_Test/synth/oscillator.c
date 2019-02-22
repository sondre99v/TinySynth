/*
 * oscillator.c
 *
 * Created: 2019-02-19 23:13:51
 *  Author: Sondre
 */ 

#include "oscillator.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#define SAMPLES_PR_WAVE 32
#define MAIN_CLOCK_FREQUENCY_HZ 19726000ULL
#define DAC_REST_VALUE 0x80
#define DAC_MIN_VALUE 0x20
#define DAC_MAX_VALUE 0xDF


static const int8_t wave_squ[SAMPLES_PR_WAVE] = {
	127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128,-128
};

static const int8_t wave_saw[SAMPLES_PR_WAVE] = {
	4,12,20,28,36,45,53,61,69,77,86,94,102,110,118,127,-128,-119,-111,-103,-95,-87,-78,-70,-62,-54,-46,-37,-29,-21,-13,-5
};

static const int8_t wave_tri[SAMPLES_PR_WAVE] = {
	-1,15,31,47,63,79,95,111,127,111,95,79,63,47,31,15,-1,-16,-32,-48,-64,-80,-96,-112,-128,-112,-96,-80,-64,-48,-32,-16
};

static const int8_t wave_sin[SAMPLES_PR_WAVE] = {
	-1,24,48,70,89,105,117,124,127,124,117,105,89,70,48,24,-1,-25,-49,-71,-90,-106,-118,-125,-128,-125,-118,-106,-90,-71,-49,-25
};


typedef struct {
	waveform_t waveform;
	uint8_t amplitude;
	uint8_t filter_value;
	int8_t prev_sample;
	uint8_t wave_index;
	uint16_t timer_period;
} osc_values_t;

volatile osc_values_t OscA = {
	.waveform = WAVE_SINE,
	.amplitude = 0,
	.filter_value = 0,
	.prev_sample = 0,
	.wave_index = 0,
	.timer_period = 2000
};

static volatile osc_values_t OscB = {
	.waveform = WAVE_SINE,
	.amplitude = 0,
	.filter_value = 0,
	.prev_sample = 0,
	.wave_index = 0,
	.timer_period = 2000
};


void osc_init()
{
	// Enable the chosen reference for the DAC
	VREF.CTRLA = (VREF_DAC0REFSEL_2V5_gc << VREF_DAC0REFSEL0_bp);
	
	// Enable the DAC and enable the output
	DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
	
	// Set output to resting value
	DAC0.DATA = DAC_REST_VALUE;
	
	// Enable the oscillators
	TCA0.SINGLE.PERBUF = OscA.timer_period;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
	
	TCB0.CCMP = OscB.timer_period;
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CTRLA = TCB_ENABLE_bm;
}


void osc_set_waveform(oscillator_t osc, waveform_t waveform)
{
	if (osc == OSCILLATOR_A) {
		OscA.waveform = waveform;
	}
	
	if (osc == OSCILLATOR_B) {
		OscB.waveform = waveform;
	}
}

uint16_t bPer = 0;

void osc_set_frequency(oscillator_t osc, uint16_t frequency_dHz)
{
	if (osc == OSCILLATOR_A) {
		OscA.timer_period = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)frequency_dHz * SAMPLES_PR_WAVE);
	}
	
	if (osc == OSCILLATOR_B) {
		OscB.timer_period = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)frequency_dHz * SAMPLES_PR_WAVE);
	}
}

void osc_set_amplitude(oscillator_t osc, uint8_t amplitude)
{
	if (osc == OSCILLATOR_A) {
		OscA.amplitude = amplitude;
	}
	
	if (osc == OSCILLATOR_B) {
		OscB.amplitude = amplitude;
	}
}

void osc_set_filter_value(oscillator_t osc, uint8_t filter_value)
{
	
}




// Interrupt handler for oscillator A
ISR(TCA0_OVF_vect)
{
	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
	
	// Compute next sample value
	volatile int16_t sample;

	switch(OscA.waveform) {
		case WAVE_SAW: 
			sample = (int16_t)wave_saw[OscA.wave_index];
			break;
		case WAVE_TRIANGLE: 
			sample = (int16_t)wave_tri[OscA.wave_index];
			break;
		case WAVE_SQUARE: 
			sample = (int16_t)wave_squ[OscA.wave_index];
			break;
		case WAVE_SINE: 
			sample = (int16_t)wave_sin[OscA.wave_index];
			break;
		default:
			sample = 0;
			break;
	}
	
	sample = (((int16_t)OscA.amplitude + 1) * sample + 128) / 256;
	
	// Write output value
	volatile int16_t change = sample - OscA.prev_sample;
	volatile int16_t current_data = (int16_t)(DAC0.DATA);
	volatile int16_t new_data = current_data + change;

	if (new_data > DAC_MAX_VALUE) {
		change -= new_data - DAC_MAX_VALUE;
	}
	else if (new_data < DAC_MIN_VALUE) {
		change += DAC_MIN_VALUE - new_data;
	}

	OscA.prev_sample += change;
	DAC0.DATA += change;
	
	// Compute current location within wave period
	OscA.wave_index++;
	if (OscA.wave_index == SAMPLES_PR_WAVE) {
		OscA.wave_index = 0;
	}
	
	TCA0.SINGLE.PER = OscA.timer_period;

	// Check for timing violation
	if (TCA0.SINGLE.CNT > TCA0.SINGLE.PER / 2) {
		PORTB.OUTCLR = (1 << 4);
	} else {
	}
}

// Interrupt handler for oscillator B
ISR(TCB0_INT_vect)
{
	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
	
	// Compute next sample value
	volatile int16_t sample;

	switch(OscB.waveform) {
		case WAVE_SAW:
			sample = (int16_t)wave_saw[OscB.wave_index];
			break;
		case WAVE_TRIANGLE:
			sample = (int16_t)wave_tri[OscB.wave_index];
			break;
		case WAVE_SQUARE:
			sample = (int16_t)wave_squ[OscB.wave_index];
			break;
		case WAVE_SINE:
			sample = (int16_t)wave_sin[OscB.wave_index];
			break;
		default:
			sample = 0;
			break;
	}
	
	sample = (((int16_t)OscB.amplitude + 1) * sample + 128) / 256;
	
	// Write output value
	volatile int16_t change = sample - OscB.prev_sample;
	volatile int16_t current_data = (int16_t)(DAC0.DATA);
	volatile int16_t new_data = current_data + change;

	if (new_data > DAC_MAX_VALUE) {
		change -= new_data - DAC_MAX_VALUE;
	}
	else if (new_data < DAC_MIN_VALUE) {
		change += DAC_MIN_VALUE - new_data;
	}

	OscB.prev_sample += change;
	DAC0.DATA += change;
	
	// Compute current location within wave period
	OscB.wave_index++;
	if (OscB.wave_index == SAMPLES_PR_WAVE) {
		OscB.wave_index = 0;

		// Sync
		//OscA.wave_index = 0;
	}
	
	TCB0.CCMP = OscB.timer_period;

	// Check for timing violation
	if (TCB0.CNT > TCB0.CCMP / 2) {
		PORTB.OUTCLR = (1 << 4);
	}
}

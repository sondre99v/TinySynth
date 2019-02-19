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

static const uint8_t wave_squ[SAMPLES_PR_WAVE] = {
	28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228
};

static const uint8_t wave_saw[SAMPLES_PR_WAVE] = {
	28,35,41,47,54,60,67,73,80,86,93,99,105,112,118,125,131,138,144,151,157,164,170,176,183,189,196,202,209,215,222,228
};

static const uint8_t wave_tri[SAMPLES_PR_WAVE] = {
	128,141,153,166,178,191,203,216,228,216,203,191,178,166,153,141,128,116,103,91,78,66,53,41,28,41,53,66,78,91,103,116
};

static const uint8_t wave_sin[SAMPLES_PR_WAVE] = {
	128,148,166,184,199,211,220,226,228,226,220,211,199,184,166,148,128,109,90,72,57,45,36,30,28,30,36,45,57,72,90,109
};


volatile oscillator_t OscillatorA = {
	.waveform = WAVE_SINE,
	.frequency_dHz = 4400,
	.amplitude = 128,
	.filter_value = 0
};

static volatile uint8_t wave_index_A = 0;
static volatile uint8_t prev_value_A = 0;

volatile oscillator_t OscillatorB = {
	.waveform = WAVE_SINE,
	.frequency_dHz = 4400,
	.amplitude = 128,
	.filter_value = 0
};

static volatile uint8_t wave_index_B = 0;
static volatile uint8_t prev_value_B = 0;


void oscillator_start(oscillator_t* osc) {
	// Enable the chosen reference for the DAC
	VREF.CTRLA = (VREF_DAC0REFSEL_2V5_gc << VREF_DAC0REFSEL0_bp);
	
	// Enable the DAC and enable the output
	DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
	
	// Enable the correct oscillator
	if (osc == &OscillatorA) {
		TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
		TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
	}
	
	if (osc == &OscillatorB) {
		TCB0.INTCTRL = TCB_CAPT_bm;
		TCB0.CTRLA = TCB_ENABLE_bm;
	}
}

void oscillator_stop(oscillator_t* osc) {
	if (osc == &OscillatorA) {
		TCA0.SINGLE.CTRLA = 0;
	}
	
	if (osc == &OscillatorB) {
		TCB0.CTRLA = 0;
	}
}


ISR(TCA0_OVF_vect) {
	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;

	// Compute current location within wave period
	wave_index_A++;
	if (wave_index_A == SAMPLES_PR_WAVE) {
		wave_index_A = 0;
	}

	uint8_t level;

	switch(OscillatorA.waveform) {
		case WAVE_SAW: 
			level = wave_saw[wave_index_A];
			break;
		case WAVE_TRIANGLE: 
			level = wave_tri[wave_index_A];
			break;
		case WAVE_SQUARE: 
			level = wave_squ[wave_index_A];
			break;
		case WAVE_SINE: 
			level = wave_sin[wave_index_A]; 
			break;
		default:
			level = 128;
			break;
	}
	
	//level = (uint8_t)((uint16_t)OscillatorA.amplitude * level) >> 8;
	
	// Write output value
	volatile int16_t change = level - prev_value_A;
	prev_value_A += change;
	DAC0.DATA += change;
	
	// Update frequency
	TCA0.SINGLE.PERBUF = MAIN_CLOCK_FREQUENCY_HZ / (OscillatorA.frequency_dHz * SAMPLES_PR_WAVE / 10);
}

ISR(TCB0_INT_vect) {
	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;

	// Compute current location within wave period
	wave_index_B++;
	if (wave_index_B == SAMPLES_PR_WAVE) {
		wave_index_B = 0;
	}

	uint8_t level;

	switch(OscillatorB.waveform) {
		case WAVE_SAW:
			level = wave_saw[wave_index_B];
			break;
		case WAVE_TRIANGLE:
			level = wave_tri[wave_index_B];
			break;
		case WAVE_SQUARE:
			level = wave_squ[wave_index_B];
			break;
		case WAVE_SINE:
			level = wave_sin[wave_index_B];
			break;
		default:
			level = 128;
			break;
	}
	
	//level = (uint8_t)((uint16_t)OscillatorB.amplitude * level) >> 8;
	
	// Write output value
	volatile int16_t change = level - prev_value_B;
	prev_value_B += change;
	DAC0.DATA += change;
	
	// Update frequency
	TCB0.CCMP = MAIN_CLOCK_FREQUENCY_HZ / (OscillatorB.frequency_dHz * SAMPLES_PR_WAVE / 10);
}

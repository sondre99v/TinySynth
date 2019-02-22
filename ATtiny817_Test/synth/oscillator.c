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
#define DAC_MIN_VALUE 0
#define DAC_MAX_VALUE 255

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


typedef struct {
	waveform_t waveform;
	uint16_t frequency_dHz;
	uint8_t amplitude;
	uint8_t filter_value;
	uint8_t prev_value;
	uint8_t wave_index;
} osc_values_t;

volatile osc_values_t OscA = {
	.waveform = WAVE_SINE,
	.frequency_dHz = 4400,
	.amplitude = 0,
	.filter_value = 0,
	.prev_value = DAC_REST_VALUE,
	.wave_index = 0
};

static volatile osc_values_t OscB = {
	.waveform = WAVE_SINE,
	.frequency_dHz = 4400,
	.amplitude = 0,
	.filter_value = 0,
	.prev_value = DAC_REST_VALUE,
	.wave_index = 0
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
	TCA0.SINGLE.PERBUF = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)OscA.frequency_dHz * SAMPLES_PR_WAVE);
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
	
	TCB0.CCMP = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)OscB.frequency_dHz * SAMPLES_PR_WAVE);
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

void osc_set_frequency(oscillator_t osc, uint16_t frequency_dHz)
{
	if (osc == OSCILLATOR_A) {
		OscA.frequency_dHz = frequency_dHz;
		TCA0.SINGLE.PERBUF = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)OscA.frequency_dHz * SAMPLES_PR_WAVE);
	}
	
	if (osc == OSCILLATOR_B) {
		OscB.frequency_dHz = frequency_dHz;
		TCB0.CCMP = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)OscB.frequency_dHz * SAMPLES_PR_WAVE);
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
	// Clear interrupt flag and set timing beacon
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
	PORTC.OUTSET = (1 << 1);
	
	// Compute next sample value (TODO: change lookup tables to signed values)
	volatile int16_t level;

	switch(OscA.waveform) {
		case WAVE_SAW: 
			level = (int16_t)wave_saw[OscA.wave_index] - 128;
			break;
		case WAVE_TRIANGLE: 
			level = (int16_t)wave_tri[OscA.wave_index] - 128;
			break;
		case WAVE_SQUARE: 
			level = (int16_t)wave_squ[OscA.wave_index] - 128;
			break;
		case WAVE_SINE: 
			level = (int16_t)wave_sin[OscA.wave_index] - 128; 
			break;
		default:
			level = 128;
			break;
	}
	
	level = (((uint16_t)OscA.amplitude + 1) * level + 128) >> 8;
	
	
	// Write output value
	volatile int16_t change = level - OscA.prev_value;
	OscA.prev_value += change;
	DAC0.DATA += change;
	
	// Compute current location within wave period
	OscA.wave_index++;
	if (OscA.wave_index == SAMPLES_PR_WAVE) {
		OscA.wave_index = 0;
	}

	// Clear timing beacon
	PORTC.OUTCLR = (1 << 1);
	
	// Check for timing violation
	if (TCA0.SINGLE.CNT > TCA0.SINGLE.PER / 2) {
		PORTB.OUTCLR = (1 << 4);
	} else {
	}
}

// Interrupt handler for oscillator B
ISR(TCB0_INT_vect)
{
	// Clear interrupt flag and set timing beacon
	TCB0.INTFLAGS = TCB_CAPT_bm;
	PORTC.OUTSET = (1 << 3);

	// Compute next sample value (TODO: change lookup tables to signed values)
	int8_t level;

	switch(OscB.waveform) {
		case WAVE_SAW:
			level = (int16_t)wave_saw[OscB.wave_index] - 128;
			break;
		case WAVE_TRIANGLE:
			level = (int16_t)wave_tri[OscB.wave_index] - 128;
			break;
		case WAVE_SQUARE:
			level = (int16_t)wave_squ[OscB.wave_index] - 128;
			break;
		case WAVE_SINE:
			level = (int16_t)wave_sin[OscB.wave_index] - 128;
			break;
		default:
			level = 128;
			break;
	}
	
	level = (((uint16_t)OscB.amplitude + 1) * level + 128) >> 8;
	
	// Write output value
	volatile int16_t change = level - OscB.prev_value;
	OscB.prev_value += change;
	DAC0.DATA += change;
	
	// Compute current location within wave period
	OscB.wave_index++;
	if (OscB.wave_index == SAMPLES_PR_WAVE) {
		OscB.wave_index = 0;
	}

	// Clear timing beacon
	PORTC.OUTCLR = (1 << 3);
	
	// Check for timing violation
	if (TCB0.CNT > TCB0.CCMP / 2) {
		PORTB.OUTCLR = (1 << 4);
	}
}

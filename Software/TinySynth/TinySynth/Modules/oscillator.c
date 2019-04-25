/*
 * oscillator.c
 *
 * Created: 19/04/19 12:17:55
 *  Author: Sondre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "oscillator.h"


#define SAMPLES_PR_WAVE 16
#define MAIN_CLOCK_FREQUENCY_HZ 10000000ULL
#define MAX_SAMPLE INT8_MAX
#define MIN_SAMPLE INT8_MIN


static const int8_t wave_sin[SAMPLES_PR_WAVE] = {
	-1, 48, 89, 117, 127, 117, 89, 48, 0, -49, -90, -118, -128, -118, -90, -49
};

static const int8_t wave_tri[SAMPLES_PR_WAVE] = {
	-1, 31, 63, 95, 127, 95, 63, 31, -1, -32, -64, -96, -128, -96, -64, -32
};

static const int8_t wave_squ[SAMPLES_PR_WAVE] = {
	-128, -128, -128, -128, -128, -128, -128, -128, 127, 127, 127, 127, 127, 127, 127, 127
};

static const int8_t wave_saw[SAMPLES_PR_WAVE] = {
	8, 25, 42, 59, 76, 93, 110, 127, -128, -111, -94, -77, -60, -43, -26, -9
};


typedef struct {
	waveform_t waveform;
	uint8_t amplitude;
	int16_t current_sample;
	uint8_t wave_index;
	uint16_t timer_period;
} oscillator_data_t;

static volatile oscillator_data_t oscillators[] = {
	{
		.waveform = WAVE_SILENCE,
		.amplitude = 0,
		.current_sample = 0,
		.wave_index = 0,
		.timer_period = 2000
	},
	{
		.waveform = WAVE_SILENCE,
		.amplitude = 0,
		.current_sample = 0,
		.wave_index = 0,
		.timer_period = 2000
	}
};

static bool oscillator_sync = false;


void oscillator_init(void)
{
	// Enable the 2.5V reference for the DAC
	VREF.CTRLA = (VREF_DAC0REFSEL_2V5_gc << VREF_DAC0REFSEL0_bp);
	
	// Set the output voltage to center
	DAC0.DATA = 128;
	
	// Enable the DAC and enable the output
	DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
	
	// Enable the oscillators
	TCA0.SINGLE.PERBUF = oscillators[(int)OSCILLATOR_A].timer_period;
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
	
	TCB0.CCMP = oscillators[(int)OSCILLATOR_B].timer_period;
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CTRLA = TCB_ENABLE_bm;
}

void oscillator_set_waveform(oscillator_t oscillator, waveform_t waveform)
{
	oscillators[(int)oscillator].waveform = waveform;
}

void oscillator_set_frequency(oscillator_t oscillator, uint16_t frequency_dHz)
{
	oscillators[(int)oscillator].timer_period = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)frequency_dHz * SAMPLES_PR_WAVE);
}

void oscillator_set_amplitude(oscillator_t oscillator, uint8_t amplitude)
{
	oscillators[(int)oscillator].amplitude = amplitude;
}

void oscillator_set_sync(bool enabled)
{
	oscillator_sync = enabled;
}

static void run_oscillator(oscillator_t oscillator) {
	// Compute next sample value
	volatile int16_t wave_sample;

	switch(oscillators[(int)oscillator].waveform) {
		case WAVE_SAW:
		wave_sample = wave_saw[oscillators[(int)oscillator].wave_index];
		break;
		case WAVE_TRIANGLE:
		wave_sample = wave_tri[oscillators[(int)oscillator].wave_index];
		break;
		case WAVE_SQUARE:
		wave_sample = wave_squ[oscillators[(int)oscillator].wave_index];
		break;
		case WAVE_SINE:
		wave_sample = wave_sin[oscillators[(int)oscillator].wave_index];
		break;
		default:
		wave_sample = 0;
		break;
	}
	
	oscillators[(int)oscillator].current_sample = 
		(((int16_t)oscillators[(int)oscillator].amplitude + 1) * wave_sample + 0x80) >> 8;
	
	volatile int16_t new_data = 
		(int16_t)oscillators[(int)OSCILLATOR_A].current_sample + oscillators[(int)OSCILLATOR_B].current_sample;
	
	if (new_data > MAX_SAMPLE) {
		new_data = MAX_SAMPLE;
	}
	else if (new_data < MIN_SAMPLE) {
		new_data = MIN_SAMPLE;
	}
	
	volatile uint8_t dac_data = (uint8_t)(0x80 + new_data);

	DAC0.DATA = dac_data;
	
	
	// Compute current location within wave period
	oscillators[(int)oscillator].wave_index++;
	if (oscillators[(int)oscillator].wave_index >= SAMPLES_PR_WAVE) {
		oscillators[(int)oscillator].wave_index = 0;
		
		// Sync oscillator B to oscillator A if enabled
		if (oscillator_sync && oscillator == OSCILLATOR_A) {
			oscillators[(int)OSCILLATOR_B].wave_index = 0;
		}
	}
}

// Interrupt handler for oscillator A
ISR(TCA0_OVF_vect)
{
	PORTC.OUTSET = (1 << 1);
	run_oscillator(OSCILLATOR_A);
	PORTC.OUTCLR = (1 << 1);

	TCA0.SINGLE.PER = oscillators[(int)OSCILLATOR_A].timer_period;

	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// Interrupt handler for oscillator B
ISR(TCB0_INT_vect)
{
	PORTC.OUTSET = (1 << 2);
	run_oscillator(OSCILLATOR_B);
	PORTC.OUTCLR = (1 << 2);
	
	TCB0.CCMP = oscillators[(int)OSCILLATOR_B].timer_period;

	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
}
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
#define MAX_SAMPLE 0xFFFF
#define MIN_SAMPLE 0x0000


static const int16_t wave_squ[SAMPLES_PR_WAVE] = {
	32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767, 32767,
	-32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768
};

static const int16_t wave_saw[SAMPLES_PR_WAVE] = {
	1057, 3171, 5285, 7399, 9513, 11627, 13741, 15855, 17969, 20083, 22197, 24311, 26425, 28539, 30653, 32767,
	-32768, -30654, -28540, -26426, -24312, -22198, -20084, -17970, -15856, -13742, -11628, -9514, -7400, -5286, -3172,	-1058
};

static const int16_t wave_tri[SAMPLES_PR_WAVE] = {
	-1, 4095, 8191, 12287, 16383, 20479, 24575, 28671, 32767, 28671, 24575, 20479, 16383, 12287, 8191, 4095,
	-1, -4096, -8192, -12288, -16384, -20480, -24576, -28672, -32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096

};

static const int16_t wave_sin[SAMPLES_PR_WAVE] = {
	-1, 6392, 12539, 18204, 23169, 27244, 30272, 32137, 32767, 32137, 30272, 27244, 23169, 18204, 12539, 6392,
	-1, -6393, -12540, -18205, -23170, -27245, -30273, -32138, -32768, -32138, -30273, -27245, -23170, -18205, -12540, -6393
};


typedef struct {
	waveform_t waveform;
	uint16_t amplitude;
	uint8_t filter_value;
	int16_t current_sample;
	uint8_t wave_index;
	uint16_t timer_period;
} osc_values_t;

static volatile osc_values_t OscA = {
	.waveform = WAVE_SINE,
	.amplitude = 0,
	.filter_value = 0,
	.current_sample = 0,
	.wave_index = 0,
	.timer_period = 2000
};

static volatile osc_values_t OscB = {
	.waveform = WAVE_SINE,
	.amplitude = 0,
	.filter_value = 0,
	.current_sample = 0,
	.wave_index = 0,
	.timer_period = 2000
};

static volatile bool sync_enabled = false;
static volatile int16_t current_DAC_data = 0;

void osc_init()
{
	// Enable the chosen reference for the DAC
	VREF.CTRLA = (VREF_DAC0REFSEL_2V5_gc << VREF_DAC0REFSEL0_bp);
	
	// Enable the DAC and enable the output
	DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
	
	// Set output to resting value
	DAC0.DATA = 128;
	
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

//uint16_t bPer = 0;

void osc_set_frequency(oscillator_t osc, uint16_t frequency_dHz)
{
	if (osc == OSCILLATOR_A) {
		OscA.timer_period = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)frequency_dHz * SAMPLES_PR_WAVE);
	}
	
	if (osc == OSCILLATOR_B) {
		OscB.timer_period = MAIN_CLOCK_FREQUENCY_HZ * 10 / ((uint32_t)frequency_dHz * SAMPLES_PR_WAVE);
	}
}

void osc_set_amplitude(oscillator_t osc, uint16_t amplitude)
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

void osc_set_sync(bool enabled) {
	sync_enabled = enabled;
}

static void run_oscillator(volatile osc_values_t* osc) {
	// Compute next sample value
	volatile int16_t wave_sample;

	switch(osc->waveform) {
		case WAVE_SAW:
		wave_sample = wave_saw[osc->wave_index];
		break;
		case WAVE_TRIANGLE:
		wave_sample = wave_tri[osc->wave_index];
		break;
		case WAVE_SQUARE:
		wave_sample = wave_squ[osc->wave_index];
		break;
		case WAVE_SINE:
		wave_sample = wave_sin[osc->wave_index];
		break;
		default:
		wave_sample = 0;
		break;
	}
	
	osc->current_sample = (((int32_t)osc->amplitude + 1) * wave_sample + 0x8000) >> 16;
	
	volatile int32_t new_data = (int32_t)OscA.current_sample + OscB.current_sample;
	
	if (new_data > MAX_SAMPLE) {
		new_data = MAX_SAMPLE;
	}
	else if (new_data < MIN_SAMPLE) {
		new_data = MIN_SAMPLE;
	}
	
	DAC0.DATA = 128 + ((new_data + 0x80) >> 8);
	
	
	// Compute current location within wave period
	osc->wave_index++;
	if (osc->wave_index >= SAMPLES_PR_WAVE) {
		osc->wave_index = 0;
		
		// Sync
		if (sync_enabled && osc == &OscB) {
			OscA.wave_index = 0;
		}
	}
}

// Interrupt handler for oscillator A
ISR(TCA0_OVF_vect)
{
	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
	
	run_oscillator(&OscA);
	
	TCA0.SINGLE.PER = OscA.timer_period;

	// Check for timing violation
	if (TCA0.SINGLE.CNT > TCA0.SINGLE.PER / 2) {
		PORTB.OUTCLR = (1 << 4);
	}
}

// Interrupt handler for oscillator B
ISR(TCB0_INT_vect)
{
	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
	
	run_oscillator(&OscB);
	
	TCB0.CCMP = OscB.timer_period;

	// Check for timing violation
	if (TCB0.CNT > TCB0.CCMP / 2) {
		PORTB.OUTCLR = (1 << 4);
	}
}

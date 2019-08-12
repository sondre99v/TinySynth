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

static int8_t wave_noise_get_sample() {
	static uint16_t lfsr = 0xACE1;
	uint8_t output = 0;

	for(int i = 0; i < 8; i++) {
		output = (output << 1) | (lfsr & 1);
		lfsr >>= 1;
		if (lfsr & 1) {
			lfsr ^= 0xB400;
		}
	}

	return *(int8_t*)&output;
}

typedef struct {
	waveform_t waveform;
	uint8_t* amplitude;
	uint8_t* note;
	uint16_t sweep_speed;
	int16_t current_sample;
	uint8_t wave_index;
	uint16_t timer_period;
	uint8_t octave;
	uint8_t detune;
} oscillator_data_t;

static oscillator_data_t oscillators[] = {
	{
		.waveform = WAVE_SILENCE,
		.amplitude = 0,
		.note = 0,
		.sweep_speed = 65535,
		.current_sample = 0,
		.wave_index = 0,
		.octave = 0,
		.timer_period = 2000,
		.detune = 0
	},
	{
		.waveform = WAVE_SILENCE,
		.amplitude = 0,
		.note = 0,
		.sweep_speed = 65535,
		.current_sample = 0,
		.wave_index = 0,
		.octave = 0,
		.timer_period = 2000,
		.detune = 0
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

	if (waveform == WAVE_SILENCE) {
		if (oscillator == OSCILLATOR_A) TCA0.SINGLE.CTRLA &= ~TCA_SINGLE_ENABLE_bm;
		if (oscillator == OSCILLATOR_B) TCB0.CTRLA &= ~TCB_ENABLE_bm;
	}
	else {
		if (oscillator == OSCILLATOR_A) TCA0.SINGLE.CTRLA = TCA_SINGLE_ENABLE_bm;
		if (oscillator == OSCILLATOR_B) TCB0.CTRLA = TCB_ENABLE_bm;
	}
}

const uint16_t freqs[] = {
	1746, 1850, 1960, 2077, 2200, 2331, 2469, 2616, 2772, 2937,
	3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939, 5233,
};

void oscillator_update(oscillator_t oscillator)
{
	oscillator_data_t* osc = &oscillators[(int)oscillator];
	
	uint16_t current = osc->timer_period;
	uint16_t target = MAIN_CLOCK_FREQUENCY_HZ * 10 / 
		((uint32_t)freqs[*(osc->note)] * SAMPLES_PR_WAVE);
	
	if (target > current) {
		if (target - current > osc->sweep_speed) {
			osc->timer_period += osc->sweep_speed;
		}
		else {
			osc->timer_period = target;
		}
	} 
	else if (target < current) {
		if (current - target > osc->sweep_speed) {
			osc->timer_period -= osc->sweep_speed;
		}
		else {
			osc->timer_period = target;
		}
	}
}

void oscillator_set_sources(oscillator_t oscillator, uint8_t* note_input, uint8_t* amplitude_input)
{
	oscillators[(int)oscillator].note = note_input;
	oscillators[(int)oscillator].amplitude = amplitude_input;
}

void oscillator_set_octave(oscillator_t oscillator, uint8_t octave)
{
	oscillators[(int)oscillator].octave = octave;
}

void oscillator_set_detune(oscillator_t oscillator, uint8_t detune)
{
	oscillators[(int)oscillator].detune = detune;
}

void oscillator_set_sync(bool enabled)
{
	oscillator_sync = enabled;
}

void oscillator_set_sweep_speed(oscillator_t oscillator, uint16_t sweep_speed)
{
	oscillators[(int)oscillator].sweep_speed = sweep_speed;
}

uint8_t _get_amplitude_for_wave(waveform_t waveform) {
	switch(waveform) {
		case WAVE_SINE: return 0xD0;
		case WAVE_TRIANGLE: return 0xC0;
		case WAVE_SQUARE: return 0x80;
		case WAVE_SAW: return 0x60;
		case WAVE_NOISE: return 0x60;
		default: return 0x00;
	}
}

uint8_t last32_A[32] = {0};
int index_A = 0;
uint8_t last32_B[32] = {0};
int index_B = 0;

static void update_dac() {
	last32_A[index_A++] = oscillators[(int)OSCILLATOR_A].current_sample;
	last32_B[index_B++] = oscillators[(int)OSCILLATOR_B].current_sample;
	
	index_A = index_A % 32;
	index_B = index_B % 32;

	volatile int16_t new_data =
		(int16_t)oscillators[(int)OSCILLATOR_A].current_sample +
		oscillators[(int)OSCILLATOR_B].current_sample;

	if (new_data > MAX_SAMPLE) {
		new_data = MAX_SAMPLE;
	}
	else if (new_data < MIN_SAMPLE) {
		new_data = MIN_SAMPLE;
	}

	volatile uint8_t dac_data = (uint8_t)(0x80 + new_data);

	DAC0.DATA = dac_data;
}

static void run_oscillator(oscillator_data_t* osc_data) {
	if (*(osc_data->amplitude) == 0) {
		osc_data->wave_index = 0;
		osc_data->current_sample = 0;
	}
	
	// Compute next sample value
	volatile int16_t wave_sample;

	switch(osc_data->waveform) {
		case WAVE_SAW:
		wave_sample = wave_saw[osc_data->wave_index];
		break;
		case WAVE_TRIANGLE:
		wave_sample = wave_tri[osc_data->wave_index];
		break;
		case WAVE_SQUARE:
		wave_sample = wave_squ[osc_data->wave_index];
		break;
		case WAVE_SINE:
		wave_sample = wave_sin[osc_data->wave_index];
		break;
		case WAVE_NOISE:
		wave_sample = wave_noise_get_sample();
		break;
		default:
		wave_sample = 0;
		break;
	}

	osc_data->current_sample = (((int32_t)*(osc_data->amplitude) + 1) * _get_amplitude_for_wave(osc_data->waveform) * wave_sample + 0x8000) >> 16;

	update_dac();

	// Compute current location within wave period
	osc_data->wave_index += (1 << osc_data->octave);
	if (osc_data->wave_index >= SAMPLES_PR_WAVE) {
		osc_data->wave_index = 0;

	}
}

// Interrupt handler for oscillator A
ISR(TCA0_OVF_vect)
{
	run_oscillator(&oscillators[(int)OSCILLATOR_A]);

	TCA0.SINGLE.PER = oscillators[(int)OSCILLATOR_A].timer_period - (((uint32_t)oscillators[(int)OSCILLATOR_A].timer_period * oscillators[(int)OSCILLATOR_A].detune) >> 8);

	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// Interrupt handler for oscillator B
ISR(TCB0_INT_vect)
{
	run_oscillator(&oscillators[(int)OSCILLATOR_B]);

	TCB0.CCMP = oscillators[(int)OSCILLATOR_B].timer_period - (((uint32_t)oscillators[(int)OSCILLATOR_B].timer_period * oscillators[(int)OSCILLATOR_B].detune) >> 8);;

	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
}
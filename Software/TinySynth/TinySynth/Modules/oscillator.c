/*
 * oscillator.c
 *
 * Created: 19/04/19 12:17:55
 *  Author: Sondre
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "oscillator.h"
#include "envelope.h"


#define WAVETABLE_LENGTH 16
#define WAVETABLE_OCTAVE 5
#define MAIN_CLOCK_FREQUENCY_HZ 10000000ULL
#define MAX_SAMPLE INT8_MAX
#define MIN_SAMPLE INT8_MIN


static const int8_t wave_sin[WAVETABLE_LENGTH] = {
	-1, 48, 89, 117, 127, 117, 89, 48, 0, -49, -90, -118, -128, -118, -90, -49
};

static const int8_t wave_tri[WAVETABLE_LENGTH] = {
	-1, 31, 63, 95, 127, 95, 63, 31, -1, -32, -64, -96, -128, -96, -64, -32
};

static const int8_t wave_squ[WAVETABLE_LENGTH] = {
	-128, -128, -128, -128, -128, -128, -128, -128, 127, 127, 127, 127, 127, 127, 127, 127
};

static const int8_t wave_saw[WAVETABLE_LENGTH] = {
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
	int8_t note_offset;
	int8_t* bend_amount;
	uint16_t sweep_speed;
	int8_t current_sample;
	uint8_t wave_index;
	uint16_t timer_period;
	uint8_t octave;
	uint8_t filter_value;
	uint8_t* filter_mod_source;
	int8_t filter_mod_amount;
} oscillator_data_t;

static oscillator_data_t oscillators[] = {
	{
		.waveform = WAVE_SILENCE,
		.amplitude = 0,
		.note = 0,
		.note_offset = 0,
		.sweep_speed = 65535,
		.current_sample = 0,
		.wave_index = 0,
		.octave = 0,
		.timer_period = 2000,
		.filter_value = 32,
		.filter_mod_source = 0,
		.filter_mod_amount = 127
	},
	{
		.waveform = WAVE_SILENCE,
		.amplitude = 0,
		.note = 0,
		.note_offset = 0,
		.sweep_speed = 65535,
		.current_sample = 0,
		.wave_index = 0,
		.octave = 0,
		.timer_period = 2000,
		.filter_value = 32,
		.filter_mod_source = 0,
		.filter_mod_amount = 127
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
	
	oscillators[0].filter_mod_source = &(ENVELOPE_3->value);
	oscillators[1].filter_mod_source = &(ENVELOPE_3->value);
}

void oscillator_set_waveform(oscillator_t oscillator, waveform_t waveform)
{
	oscillators[(int)oscillator].waveform = waveform;
}

const uint16_t periods[] = {
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (2616UL * WAVETABLE_LENGTH), // C  - 261.6 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (2772UL * WAVETABLE_LENGTH), // C# - 277.2 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (2937UL * WAVETABLE_LENGTH), // D  - 293.7 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (3111UL * WAVETABLE_LENGTH), // Eb - 311.1 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (3296UL * WAVETABLE_LENGTH), // E  - 329.6 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (3492UL * WAVETABLE_LENGTH), // F  - 349.2 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (3700UL * WAVETABLE_LENGTH), // F# - 370.0 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (3920UL * WAVETABLE_LENGTH), // G  - 392.0 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (4153UL * WAVETABLE_LENGTH), // G# - 415.3 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (4400UL * WAVETABLE_LENGTH), // A  - 440.0 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (4662UL * WAVETABLE_LENGTH), // Bb - 466.2 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (4939UL * WAVETABLE_LENGTH), // B  - 493.9 Hz
	MAIN_CLOCK_FREQUENCY_HZ * 10 / (5233UL * WAVETABLE_LENGTH)  // C  - 523.3 Hz (Used for interpolation)
};

static uint16_t modulate16(uint16_t base_value, uint16_t mod_value, int8_t mod_amount) {
	
	int32_t mod = (mod_value * mod_amount + mod_value * (mod_amount > 0) + 0x7F) >> 7;
	
	if (base_value + mod > 65535) {
		return 65535;
	}
	else if (base_value + mod < 0) {
		return 0;
	}
	else {
		return base_value + mod;
	}
}

void oscillator_update(oscillator_t oscillator)
{
	oscillator_data_t* osc = &oscillators[(int)oscillator];
	
	uint8_t note = *(osc->note) + osc->note_offset;
	
	int8_t bend_amount = osc->bend_amount ? *(osc->bend_amount) : 0;
	
	if (bend_amount < 0 && note > 0) {
		bend_amount += 128;
		note -= 1;
	}
	
	uint8_t scale_note = note % 12;
	uint8_t octave = note / 12;
	
	uint16_t period0 = periods[scale_note];
	uint16_t period1 = periods[scale_note + 1];
	
	uint16_t period = modulate16(period1, period0 - period1, 127 - bend_amount);
	
	if (octave >= WAVETABLE_OCTAVE) {
		osc->octave = octave;
	}
	else {
		period <<= WAVETABLE_OCTAVE - octave;
		osc->octave = WAVETABLE_OCTAVE;
	}
	
	osc->timer_period = period;
}

void oscillator_set_sources(oscillator_t oscillator, uint8_t* note_input, int8_t* bend_input, uint8_t* amplitude_input)
{
	oscillators[(int)oscillator].note = note_input;
	oscillators[(int)oscillator].bend_amount = bend_input;
	oscillators[(int)oscillator].amplitude = amplitude_input;
}

void oscillator_set_note_offset(oscillator_t oscillator, uint8_t note_offset)
{
	oscillators[(int)oscillator].note_offset = note_offset;
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
		case WAVE_SAW: return 0xC0;
		case WAVE_NOISE: return 0x60;
		default: return 0x00;
	}
}

static void update_dac() {
	// Combine oscA and oscB
	volatile int16_t new_data =
		(int16_t)oscillators[(int)OSCILLATOR_A].current_sample +
		oscillators[(int)OSCILLATOR_B].current_sample;

	// Debug attenuation for comfort
	new_data /= 16;

	// Clamp output
	if (new_data > MAX_SAMPLE) {
		new_data = MAX_SAMPLE;
	}
	else if (new_data < MIN_SAMPLE) {
		new_data = MIN_SAMPLE;
	}

	// Apply output to DAC
	DAC0.DATA = (uint8_t)(0x80 + new_data);
}

#define SCALE(v, x) (( (int16_t)(v) * (x) + (v) + 0x80) >> 8)

static uint8_t modulate(uint8_t base_value, uint8_t mod_value, int8_t mod_amount) {
	
	int16_t mod = (mod_value * mod_amount + mod_value * (mod_amount > 0) + 0x7F) >> 7;
	
	if (base_value + mod > 255) {
		return 255;
	}
	else if (base_value + mod < 0) {
		return 0;
	}
	else {
		return base_value + mod;
	}
}

static void run_oscillator(oscillator_data_t* osc_data) {
	if (*(osc_data->amplitude) == 0) {
		osc_data->wave_index = 0;
		osc_data->current_sample = 0;
	}
	
	// Compute next sample value
	volatile int8_t wave_sample;

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

	// Combine set amplitude and waveform amplitude correction
	uint8_t amp = *osc_data->amplitude;
	uint8_t wave_amp = _get_amplitude_for_wave(osc_data->waveform);
	amp = SCALE(amp, wave_amp);

	// Compute new sample
	int8_t new_sample = SCALE(wave_sample, amp);

	// Apply filter to compute actual sample
	volatile uint8_t filter = modulate(osc_data->filter_value, *(osc_data->filter_mod_source), osc_data->filter_mod_amount);

	osc_data->current_sample = ((int16_t)new_sample * filter + (int16_t)osc_data->current_sample * (0x100 - (uint8_t)filter)) >> 8;


	update_dac();

	// Compute current location within wave period
	osc_data->wave_index += (1 << (osc_data->octave - WAVETABLE_OCTAVE));
	if (osc_data->wave_index >= WAVETABLE_LENGTH) {
		osc_data->wave_index = 0;

	}
}

// Interrupt handler for oscillator A
ISR(TCA0_OVF_vect)
{
	run_oscillator(&oscillators[(int)OSCILLATOR_A]);

	TCA0.SINGLE.PER = oscillators[(int)OSCILLATOR_A].timer_period + wave_noise_get_sample() / 16;

	volatile __attribute__((unused)) int16_t margin = TCA0.SINGLE.PER - TCA0.SINGLE.CNT;

	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

// Interrupt handler for oscillator B
ISR(TCB0_INT_vect)
{
	run_oscillator(&oscillators[(int)OSCILLATOR_B]);

	TCB0.CCMP = oscillators[(int)OSCILLATOR_B].timer_period + wave_noise_get_sample() / 16;

	volatile __attribute__((unused)) int16_t margin = TCB0.CCMP - TCB0.CNT;

	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;
}
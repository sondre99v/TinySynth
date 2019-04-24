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
#define MAX_SAMPLE INT16_MAX
#define MIN_SAMPLE INT16_MIN


static const int16_t wave_sin[SAMPLES_PR_WAVE] = {
	-1,  6392,  12539,  18204,  23169,  27244,  30272,  32137,  32767,  32137,  30272,  27244,  23169,  18204,  12539,  6392,
	-1, -6393, -12540, -18205, -23170, -27245, -30273, -32138, -32768, -32138, -30273, -27245, -23170, -18205, -12540, -6393
};

static const int16_t wave_tri[SAMPLES_PR_WAVE] = {
	-1,  4095,  8191,  12287,  16383,  20479,  24575,  28671,  32767,  28671,  24575,  20479,  16383,  12287,  8191,  4095,
	-1, -4096, -8192, -12288, -16384, -20480, -24576, -28672, -32768, -28672, -24576, -20480, -16384, -12288, -8192, -4096
};

static const int16_t wave_squ[SAMPLES_PR_WAVE] = {
	 32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,  32767,
	-32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768, -32768
};

static const int16_t wave_saw[SAMPLES_PR_WAVE] = {
	  1057,   3171,   5285,   7399,   9513,  11627,  13741,  15855,  17969,  20083,  22197,  24311, 26425, 28539, 30653, 32767,
	-32768, -30654, -28540, -26426, -24312, -22198, -20084, -17970, -15856, -13742, -11628, -9514,  -7400, -5286, -3172, -1058
};


typedef struct {
	waveform_t waveform;
	uint16_t amplitude;
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

void oscillator_set_amplitude(oscillator_t oscillator, uint16_t amplitude)
{
	oscillators[(int)oscillator].amplitude = amplitude;
}

void oscillator_set_sync(bool enabled)
{
	oscillator_sync = enabled;
}


uint8_t index = 0;
volatile uint8_t log[64] = {0};

static void run_oscillator(oscillator_t oscillator) {
	// Compute next sample value
	volatile int16_t wave_sample;

	switch(oscillators[(int)oscillator].waveform) {
		case WAVE_SAW:
		wave_sample = wave_saw[oscillators[(int)oscillator].wave_index*2];
		break;
		case WAVE_TRIANGLE:
		wave_sample = wave_tri[oscillators[(int)oscillator].wave_index*2];
		break;
		case WAVE_SQUARE:
		wave_sample = wave_squ[oscillators[(int)oscillator].wave_index*2];
		break;
		case WAVE_SINE:
		wave_sample = wave_sin[oscillators[(int)oscillator].wave_index*2];
		break;
		default:
		wave_sample = 0;
		break;
	}
	
	oscillators[(int)oscillator].current_sample = 
		(((int32_t)oscillators[(int)oscillator].amplitude + 1) * wave_sample + 0x8000) >> 16;
	
	volatile int32_t new_data = 
		(int32_t)oscillators[(int)OSCILLATOR_A].current_sample + oscillators[(int)OSCILLATOR_B].current_sample;
	
	if (new_data > MAX_SAMPLE) {
		new_data = MAX_SAMPLE;
	}
	else if (new_data < MIN_SAMPLE) {
		new_data = MIN_SAMPLE;
	}
	
	volatile uint8_t dac_data = (uint8_t)(0x80 + ((new_data + 0x0080) >> 8));
	log[(index++) & 0x3F] = dac_data;


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
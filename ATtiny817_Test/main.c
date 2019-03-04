/*
 * ATtiny817_Test.c
 *
 * Created: 2019-02-16 23:18:48
 * Author : Sondre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>
#include <stdint.h>
#include <stdbool.h>

#include "synth/oscillator.h"

#define MAIN_CLOCK_FREQUENCY_HZ 19726000ULL

#define TIME_TIMER_PERIOD 2500 // Gives 4ms resolution with 20MHz clock
#define TIME_TIMER_PERIOD_US (1000000ULL / (MAIN_CLOCK_FREQUENCY_HZ / 32 / TIME_TIMER_PERIOD))

volatile uint32_t time_us = 0;


int32_t ampl = 0;
bool note_on = false;

ISR(TCD0_OVF_vect) {
	TCD0.INTFLAGS = 0x01;
	
	// Increment time
	time_us += TIME_TIMER_PERIOD_US;
	
	//envelope_update(TIME_TIMER_PERIOD_US);
	
	if(!note_on && ampl > 0) {
		ampl -= 200;
		if (ampl < 0) {
			ampl = 0;
		}
	}
	
	osc_set_amplitude(OSCILLATOR_A, ampl);
	osc_set_amplitude(OSCILLATOR_B, ampl);
}

void wait() {
	for (volatile int i = 0; i < 10000; i++) { }
}

int main(void)
{
	// Disable main clock prescaler to get full 20MHz speed
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 0;
	
	PORTB.PIN5CTRL = PORT_PULLUPEN_bm;
	
	time_us = 0;
	
	int freqs[] = {
		1746, 1850, 1960, 2077, 2200, 2331, 2469, 2616, 2772, 2937, 
		3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939, 5233
	};
	
	uint8_t note_thresholds[] = {
		6, 19, 32, 45, 58, 70, 83, 96, 109, 122, 
		134, 147, 160, 173, 186, 198, 211, 224, 237, 250};

	// Setup TCD0 to give low resolution (4ms) timer
	TCD0.CMPBCLR = TIME_TIMER_PERIOD;
	TCD0.INTCTRL = TCD_OVF_bm;
	TCD0.CTRLA = TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;
	
	PORTB.DIRSET = (1 << 4);
	
	PORTB.OUTSET = (1 << 4);

	
	PORTC.DIRSET = 0xF;
	PORTC.OUTSET = 0xF;
	PORTB.DIRCLR = 0xF;
	PORTB.PIN0CTRL |= PORT_PULLUPEN_bm;
	PORTB.PIN1CTRL |= PORT_PULLUPEN_bm;
	PORTB.PIN2CTRL |= PORT_PULLUPEN_bm;
	PORTB.PIN3CTRL |= PORT_PULLUPEN_bm;


	osc_init();

	sei();
	
	osc_set_waveform(OSCILLATOR_A, WAVE_SINE);
	osc_set_waveform(OSCILLATOR_B, WAVE_SINE);
	osc_set_frequency(OSCILLATOR_A, 4400);
	osc_set_frequency(OSCILLATOR_B, 4400);
	
	ADC0.CTRLB = ADC_SAMPCAP_bm;
	ADC0.CTRLC = ADC_PRESC_DIV64_gc | ADC_REFSEL_VDDREF_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
	ADC0.CTRLA = ADC_RESSEL_8BIT_gc | ADC_ENABLE_bm;

	ADC0.COMMAND = ADC_STCONV_bm;

	bool fifth = false;
	uint8_t octA = 0;
	uint8_t octB = 0;

    while (1)
    {
		// Read switch matrix
		volatile uint16_t sw = 0;
		
		PORTC.OUTSET = 0x0E;
		PORTC.OUTCLR = 0x01;
		wait();
		sw |= (PORTB.IN & 0x0F) << 0;
		PORTC.OUTSET = 0x0D;
		PORTC.OUTCLR = 0x02;
		wait();
		sw |= (PORTB.IN & 0x0F) << 4;
		PORTC.OUTSET = 0x0B;
		PORTC.OUTCLR = 0x04;
		wait();
		sw |= (PORTB.IN & 0x0F) << 8;
		PORTC.OUTSET = 0x07;
		PORTC.OUTCLR = 0x08;
		wait();
		sw |= (PORTB.IN & 0x0F) << 12;

		sw = ~sw;
		
		int i;
		int switch_index1 = -1;
		for(i = 0; i < 8; i++) {
			if (sw & (1 << i)) {
				switch_index1 = i;
				break;
			}
		}
		int switch_index2 = -1;
		for(i = 0; i < 8; i++) {
			if ((sw >> 8) & (1 << i)) {
				switch_index2 = i;
				break;
			}
		}


		// Read keyboard
		while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) { }
		volatile uint8_t adc_value = ADC0.RES;
		ADC0.INTFLAGS = ADC_RESRDY_bm;
		ADC0.COMMAND = ADC_STCONV_bm;
		
		/*int16_t note_index = -1;
		
		while(note_index < 19 && adc_value > note_thresholds[note_index + 1]) {
			note_index++;
		}*/
		
		int16_t note_index = 19;
		while(note_index > -1 && note_thresholds[note_index] > adc_value) {
			note_index--;
		}
		
		// Set note output
		if (note_index != -1) {
			osc_set_frequency(OSCILLATOR_A, (freqs[note_index % 20] + (fifth ? (freqs[note_index % 20] >> 1) : 0)) >> octA);
			osc_set_frequency(OSCILLATOR_B, (freqs[note_index % 20]) >> octB);
			
			note_on = true;
			ampl = 65535;
		} else {
			note_on = false;
		}
		
		
		// Set waveform
		switch(switch_index2) {
			case 0: osc_set_waveform(OSCILLATOR_A, WAVE_SINE); break;
			case 1: osc_set_waveform(OSCILLATOR_A, WAVE_SQUARE); break;
			case 2: osc_set_waveform(OSCILLATOR_A, WAVE_SAW); break;
			case 3: osc_set_waveform(OSCILLATOR_A, WAVE_TRIANGLE); break;
			case 4: osc_set_waveform(OSCILLATOR_B, WAVE_SINE); break;
			case 5: osc_set_waveform(OSCILLATOR_B, WAVE_SQUARE); break;
			case 6: osc_set_waveform(OSCILLATOR_B, WAVE_SAW); break;
			case 7: osc_set_waveform(OSCILLATOR_B, WAVE_TRIANGLE); break;
		}
		
		switch(switch_index1) {
			case 0: osc_set_sync(false); break;
			case 1: osc_set_sync(true); break;
			case 2: fifth = false; break;
			case 3: fifth = true; break;
			case 4: octA = 0; break;
			case 5: octA = 1; break;
			case 6: octB = 0; break;
			case 7: octB = 1; break;
		}
    }
}


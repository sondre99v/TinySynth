/*
 * TinySynth.c
 *
 * Created: 12/04/19 17:22:15
 * Author : Sondre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Modules/oscillator.h"

#define TIME_TIMER_PERIOD 6250 // Gives 100Hz frequency with 20MHz clock and 32x prescaler


ISR(TCD0_OVF_vect) {
	TCD0.INTFLAGS = 0x01;
	
	//envelope_update(TIME_TIMER_PERIOD_US);
	
	///osc_set_amplitude(OSCILLATOR_A, ampl);
	//osc_set_amplitude(OSCILLATOR_B, ampl);
}

int main(void)
{
	// Disable main clock prescaler to get 10MHz speed
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 1;
	
	// Enable "sync" and "tune" buttons, and the "sync" LED
	PORTC.DIRSET = 0x7;
	PORTC.OUT = 0x1;
	
	PORTB.PIN0CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN1CTRL = PORT_PULLUPEN_bm;


	// Enable ADC
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV64_gc;
	ADC0.CTRLA = ADC_RESSEL_8BIT_gc | ADC_ENABLE_bm;

	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
	

	
	uint16_t freqs[] = {
		1746, 1850, 1960, 2077, 2200, 2331, 2469, 2616, 2772, 2937,
		3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939, 5233
	};
	

	// Setup TCD0 to give a 100Hz interrupt
	//TCD0.CMPBCLR = TIME_TIMER_PERIOD;
	//TCD0.INTCTRL = TCD_OVF_bm;
	//TCD0.CTRLA = TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;


	oscillator_init();
	
	oscillator_set_amplitude(OSCILLATOR_A, 0x30);
	oscillator_set_amplitude(OSCILLATOR_B, 0x30);
	oscillator_set_waveform(OSCILLATOR_A, WAVE_SILENCE);
	oscillator_set_waveform(OSCILLATOR_B, WAVE_SILENCE);

	sei();

	while (1) 
    {
	    if (!(PORTB.IN & 0x1)) {
		    oscillator_set_amplitude(OSCILLATOR_A, 0x80);
	    }
	    
	    if (!(PORTB.IN & 0x2)) {
		    oscillator_set_amplitude(OSCILLATOR_A, 0x10);
	    }
		
		ADC0.INTFLAGS = ADC_RESRDY_bm;
	    ADC0.COMMAND = ADC_STCONV_bm;
	    while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) { }
	    volatile uint8_t adc_value = ADC0.RES;

		int8_t index = 19 - ((20 * adc_value + 0x80) >> 8);

		uint8_t play = 1;
		if (play && index >= 0) {
			//PORTC.OUT = 0;
			oscillator_set_waveform(OSCILLATOR_A, WAVE_SAW);
			oscillator_set_waveform(OSCILLATOR_B, WAVE_TRIANGLE);
			oscillator_set_frequency(OSCILLATOR_A, freqs[index]);
			oscillator_set_frequency(OSCILLATOR_B, freqs[index]+freqs[index]/2);
		} else {
			//PORTC.OUT = 1;
			oscillator_set_waveform(OSCILLATOR_A, WAVE_SILENCE);
			oscillator_set_waveform(OSCILLATOR_B, WAVE_SILENCE);
		}
    }
}


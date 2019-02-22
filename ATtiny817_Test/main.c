/*
 * ATtiny817_Test.c
 *
 * Created: 2019-02-16 23:18:48
 * Author : Sondre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "synth/oscillator.h"

#define MAIN_CLOCK_FREQUENCY_HZ 19726000ULL

#define TIME_TIMER_PERIOD 2500 // Gives 4ms resolution with 20MHz clock
#define TIME_TIMER_PERIOD_US (1000000ULL / (MAIN_CLOCK_FREQUENCY_HZ / 32 / TIME_TIMER_PERIOD))

volatile uint32_t time_us = 0;


ISR(TCD0_OVF_vect) {
	// Increment time
	time_us += TIME_TIMER_PERIOD_US;
	TCD0.INTFLAGS = 0x01;
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
	
	int freqs[] = {262, 294, 330, 349, 392, 440, 494, 523};

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

    while (1)
    {
		while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) { }
		volatile uint8_t adc_value = ADC0.RES;
		ADC0.INTFLAGS = ADC_RESRDY_bm;
		ADC0.COMMAND = ADC_STCONV_bm;

		
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

		if (switch_index1 != -1) {
			osc_set_frequency(OSCILLATOR_A, freqs[switch_index1 % 8] * 10 * (50UL + adc_value) / 100);
			osc_set_frequency(OSCILLATOR_B, freqs[switch_index1 % 8] * 10);
			osc_set_amplitude(OSCILLATOR_A, 50);
			osc_set_amplitude(OSCILLATOR_B, 50);
		}
		else {
			osc_set_amplitude(OSCILLATOR_A, 0);
			osc_set_amplitude(OSCILLATOR_B, 0);
		}
		/*
		if (switch_index2 != -1) {
			osc_set_frequency(OSCILLATOR_B, freqs[switch_index2 % 8] * 10);
			osc_set_amplitude(OSCILLATOR_B, 50);
		}
		else {
			osc_set_amplitude(OSCILLATOR_B, 0);
		}*/
    }
}


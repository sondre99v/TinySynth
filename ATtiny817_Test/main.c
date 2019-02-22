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
	
	PORTC.DIRSET = (1 << 1) | (1 << 3);
	PORTC.DIRSET = (1 << 1) | (1 << 3);
	PORTB.DIRSET = (1 << 4);
	
	PORTB.OUTSET = (1 << 4);


	osc_init();

	sei();
	
	osc_set_waveform(OSCILLATOR_A, WAVE_SQUARE);
	osc_set_waveform(OSCILLATOR_B, WAVE_SINE);
	osc_set_frequency(OSCILLATOR_A, 4400);
	osc_set_frequency(OSCILLATOR_B, 4400);
	
    while (1)
    {
	    if (!(PORTB.IN & (1<<5))) {
		    osc_set_amplitude(OSCILLATOR_A, 100);
		}
		else {
			osc_set_amplitude(OSCILLATOR_A, 0);
		}

		if (!(PORTC.IN & (1<<5))) {
			osc_set_amplitude(OSCILLATOR_B, 100);
		}
		else {
			osc_set_amplitude(OSCILLATOR_B, 0);
		}
    }
}


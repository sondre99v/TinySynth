/*
 * ATtiny817_Test.c
 *
 * Created: 2019-02-16 23:18:48
 * Author : Sondre
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "peripheral/dac.h"

#define MAIN_CLOCK_FREQUENCY_HZ 19726000ULL

#define TIME_TIMER_PERIOD 2500 // Gives 4ms resolution with 20MHz clock
#define TIME_TIMER_PERIOD_US (1000000ULL / (MAIN_CLOCK_FREQUENCY_HZ / 32 / TIME_TIMER_PERIOD))

#define SAMPLES_PR_WAVE 32

volatile uint32_t time_us = 0;
volatile uint16_t current_frequency = 100;
volatile uint8_t current_wave_index = 0;
volatile uint8_t wave_pos = 0;
volatile uint8_t filter_aggr = 0;

uint8_t wave_squ[] = {
	28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,28,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228,228
};

uint8_t wave_saw[] = {
	28,35,41,47,54,60,67,73,80,86,93,99,105,112,118,125,131,138,144,151,157,164,170,176,183,189,196,202,209,215,222,228
};

uint8_t wave_tri[] = {
	128,141,153,166,178,191,203,216,228,216,203,191,178,166,153,141,128,116,103,91,78,66,53,41,28,41,53,66,78,91,103,116

};

uint8_t wave_sin[] = {
	128,148,166,184,199,211,220,226,228,226,220,211,199,184,166,148,128,109,90,72,57,45,36,30,28,30,36,45,57,72,90,109
};

volatile uint8_t filtered = 0;

volatile uint8_t prevA = 0;

ISR(TCA0_OVF_vect) {
	// Set timing beacon
	PORTC.OUTSET = (1 << 1);

	// Clear interrupt flag
	TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;



	// Compute current location within wave period
	wave_pos++;
	if (wave_pos == SAMPLES_PR_WAVE) {
		wave_pos = 0;
	}

	uint8_t level;

	switch(current_wave_index) {
		case 0: level = 2 * wave_saw[wave_pos] / 10; break;
		case 1: level = 2 * wave_tri[wave_pos] / 10; break;
		case 2: level = 2 * wave_squ[wave_pos] / 10; break;
		case 3: level = 2 * wave_sin[wave_pos] / 10; break;
	}
	
	volatile int16_t change = level - prevA;
	prevA += change;
	DAC0.DATA += change;
	
	// Set output voltage
	//filtered = (uint8_t)(((uint16_t)filtered * filter_aggr + (uint16_t)level * (16-filter_aggr)) >> 4);
	//dac_set_data(filtered);



	// Clear timing beacon
	PORTC.OUTCLR = (1 << 1);
}

volatile uint8_t wave_posB = 0;
volatile uint8_t prevB = 0;
ISR(TCB0_INT_vect) {
	// Set timing beacon
	PORTC.OUTSET = (1 << 1);

	// Clear interrupt flag
	TCB0.INTFLAGS = TCB_CAPT_bm;

	// Compute current location within wave period
	wave_posB++;
	if (wave_posB == SAMPLES_PR_WAVE) {
		wave_posB = 0;
	}

	uint8_t level;

	switch(current_wave_index) {
		case 0: level = 4 * wave_saw[wave_posB] / 10; break;
		case 1: level = 4 * wave_tri[wave_posB] / 10; break;
		case 2: level = 4 * wave_squ[wave_posB] / 10; break;
		case 3: level = 4 * wave_sin[wave_posB] / 10; break;
	}
	
	volatile int16_t change = level - prevB;
	prevB += change;
	DAC0.DATA += change;
	

	// Clear timing beacon
	PORTC.OUTCLR = (1 << 1);
}


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
	
	
	dac_init(REF_2_5V);
	DAC0.DATA = 0;
	time_us = 0;
	
	PORTC.DIR |= (1 << 1);
	PORTB.DIR |= (1 << 4);
	PORTB.PIN5CTRL = PORT_PULLUPEN_bm;
	
	int freqs[] = {262, 294, 330, 349, 392, 440, 494, 523};

	// Setup TCA0 as tone generator
	TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
	TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc | TCA_SINGLE_ENABLE_bm;
	
	// Setup TCB0 as tone generator
	TCB0.INTCTRL = TCB_CAPT_bm;
	TCB0.CTRLA = TCB_ENABLE_bm;
	
	// Setup TCD0 to give low resolution (4ms) timer
	TCD0.CMPBCLR = TIME_TIMER_PERIOD;
	TCD0.INTCTRL = TCD_OVF_bm;
	TCD0.CTRLA = TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;

	sei();

    while (1)
    {
	    if (!(PORTB.IN & (1<<5))) {
		    DAC0.CTRLA = 0;
		    DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
		    PORTB.OUTSET = (1<<4);
			current_frequency = 392;
		} else if (!(PORTC.IN & (1<<5))) {
			DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
			PORTB.OUTSET = (1<<4);
			current_frequency = 349;
		} else {
		    DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
		    DAC0.CTRLA = 0;
		    PORTB.OUTCLR = (1<<4);
	    }
		
		//current_frequency = 440;//freqs[(int)(time_us / 200000) % 8];
		current_wave_index = 2;//(int)(time_us / 1600000) % 4;

		//filter_aggr = wave_sin[(int)(time_us / 30000) % 32] >> 4;

	    TCA0.SINGLE.PERBUF = MAIN_CLOCK_FREQUENCY_HZ / (22 * current_frequency / 14 * SAMPLES_PR_WAVE);
		
		TCB0.CCMP = MAIN_CLOCK_FREQUENCY_HZ / (current_frequency / 2 * SAMPLES_PR_WAVE);
		
    }
}


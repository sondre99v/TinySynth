/*
 * keyboard.c
 *
 * Created: 13/04/19 18:20:08
 *  Author: Sondre
 */ 

#include "keyboard.h"

#include <avr/io.h>


volatile static uint8_t note_value;
volatile static uint8_t gate_value;


void keyboard_init(void)
{
	ADC0.CTRLA = ADC_RESSEL_8BIT_gc | ADC_ENABLE_bm;
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV64_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
}

void keyboard_update(void)
{
	ADC0.INTFLAGS = ADC_RESRDY_bm;
	ADC0.COMMAND = ADC_STCONV_bm;
	while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) { }
	volatile uint8_t adc_value = ADC0.RES;

	int8_t index = 19 - ((20 * adc_value + 0x80) >> 8);
	
	if (index >= 0) {
		note_value = index;
		gate_value = 1;
	}
	else {
		gate_value = 0;
	}
}


uint8_t keyboard_get_note(void)
{
	return note_value;
}


uint8_t keyboard_get_gate(void)
{
	return gate_value;
}

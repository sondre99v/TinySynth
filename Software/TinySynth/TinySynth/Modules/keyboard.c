/*
 * keyboard.c
 *
 * Created: 13/04/19 18:20:08
 *  Author: Sondre
 */ 

#include "keyboard.h"

#include <avr/io.h>


static uint8_t note_value;
static uint8_t gate_value;


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
	
	uint8_t adc_value = ADC0.RES;

	// convert adc_value -> note_number

	if (adc_value < 255) {
		gate_value = 1;
	}

	note_value = adc_value;
}


uint8_t keyboard_get_note(void)
{
	return note_value;
}


uint8_t keyboard_get_gate(void)
{
	return gate_value;
}

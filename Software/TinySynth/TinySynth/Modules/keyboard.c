/*
 * keyboard.c
 *
 * Created: 13/04/19 18:20:08
 *  Author: Sondre
 */

#include "keyboard.h"

#include <avr/io.h>


volatile static uint8_t note_value;
volatile uint8_t gate_value;
volatile static uint8_t gate_pulse_timer = 0;

uint8_t keyboard_note;


void keyboard_init(void)
{
	ADC0.CTRLA = ADC_RESSEL_8BIT_gc | ADC_ENABLE_bm;
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV64_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
}

static const uint8_t thresholds[20] = {
	8, 24, 37, 49, 62, 76, 88, 100, 113, 126, 139, 152, 164, 177, 191, 202, 213, 224, 236, 249
};

void keyboard_update(void)
{
	ADC0.INTFLAGS = ADC_RESRDY_bm;
	ADC0.COMMAND = ADC_STCONV_bm;
	while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) { }
	volatile uint8_t adc_value = ADC0.RES;

	int8_t index = 0;

	while(index < 20 && adc_value > thresholds[index]) {
		index++;
	}

	index = 19 - index;

	if (index >= 0) {
		note_value = index;
		gate_value = 1;
	}
	else {
		gate_value = 0;
	}

	if (gate_pulse_timer > 0) {
		gate_value = 1;
		gate_pulse_timer--;
	}
	
	keyboard_note = note_value;
}


uint8_t keyboard_get_note(void)
{
	return note_value;
}


uint8_t keyboard_get_gate(void)
{
	return gate_value;
}

void keyboard_pulse_gate(void)
{
	gate_pulse_timer = 100;
}
/*
 * keyboard.c
 *
 * Created: 13/04/19 18:20:08
 *  Author: Sondre
 */

#include "keyboard.h"

#include <avr/io.h>


#define SLIDE_SPEED 12

typedef struct {
	keyboard_t public;
	uint8_t gate_pulse_timer;
	bool slide_enabled;
} keyboard_data_t;

static keyboard_data_t KEYBOARD_DATA_1 = {.public.note_value = 60};

keyboard_t* const KEYBOARD_1 = (keyboard_t*)&KEYBOARD_DATA_1;

void keyboard_init(keyboard_t* keyboard)
{
	ADC0.CTRLA = ADC_RESSEL_8BIT_gc | ADC_ENABLE_bm;
	ADC0.CTRLC = ADC_SAMPCAP_bm | ADC_REFSEL_VDDREF_gc | ADC_PRESC_DIV64_gc;
	ADC0.MUXPOS = ADC_MUXPOS_AIN3_gc;
}

static const uint8_t thresholds[20] = {
	8, 24, 37, 49, 62, 76, 88, 100, 113, 126, 139, 152, 164, 177, 191, 202, 213, 224, 236, 249
};

uint16_t current_notex128 = 60 * 128;
uint16_t target_notex128 = 60 * 128;


void keyboard_update(keyboard_t* keyboard)
{
	static uint8_t prev_adc = 255;
	
	uint8_t prev_gate = keyboard->gate_value;

	keyboard_data_t* data = (keyboard_data_t*)keyboard;

	ADC0.INTFLAGS = ADC_RESRDY_bm;
	ADC0.COMMAND = ADC_STCONV_bm;
	while (!(ADC0.INTFLAGS & ADC_RESRDY_bm)) { }
		
	volatile uint8_t adc_value = ADC0.RES;

	volatile int8_t key = 0;

	while(key < 20 && adc_value > thresholds[key]) {
		key++;
	}

	key = 19 - key;
	
	if (key >= 0) {
		target_notex128 = (key + 53) << 7;
	}

	if (data->slide_enabled) {
		if ((current_notex128 + SLIDE_SPEED) < target_notex128) {
			current_notex128 += SLIDE_SPEED;
		}
		else if ((current_notex128 - SLIDE_SPEED) > target_notex128) {
			current_notex128 -= SLIDE_SPEED;
		}
		else {
			current_notex128 = target_notex128;
		}
	}
	else {
		current_notex128 = target_notex128;
	}


	if (key >= 0) {
		data->public.gate_value = 1;
		data->public.note_value = current_notex128 >> 7;
		data->public.bend_value = current_notex128 & 0x7F;
	}
	else {
		data->public.gate_value = 0;
	}

	if (data->gate_pulse_timer > 0) {
		data->public.gate_value = 1;
		data->gate_pulse_timer--;
	}

	data->public.trigger_value = data->public.gate_value && !prev_gate;
}

void keyboard_pulse_gate(keyboard_t* keyboard)
{
	keyboard_data_t* data = (keyboard_data_t*)keyboard;
	data->gate_pulse_timer = 100;
}

void keyboard_enable_slide(keyboard_t* keyboard)
{
	keyboard_data_t* data = (keyboard_data_t*)keyboard;
	data->slide_enabled = true;
}

void keyboard_disable_slide(keyboard_t* keyboard)
{
	keyboard_data_t* data = (keyboard_data_t*)keyboard;
	data->slide_enabled = false;
}

/*
 * envelope.c
 *
 * Created: 13/04/19 18:31:53
 *  Author: Sondre
 */ 

#include "envelope.h"
#include "keyboard.h"


static uint8_t envelope_value;

uint8_t envelope_rise_speed;
uint8_t envelope_fall_speed;


void envelope_init(void)
{
	envelope_value = 0;
}


void envelope_update(void)
{
	volatile uint8_t gate = keyboard_get_gate();
	
	if (gate && envelope_value < 255) {
		if (envelope_value > 255 - envelope_rise_speed) {
			envelope_value = 255;
		}
		else {
			envelope_value += envelope_rise_speed;
		}
	}
	
	if (!gate && envelope_value > 0) {
		if (envelope_value < envelope_fall_speed) {
			envelope_value = 0;
		} 
		else {
			envelope_value -= envelope_fall_speed;
		}
	}
}


uint8_t envelope_get_value(void)
{
	return envelope_value;
}

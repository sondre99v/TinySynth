/*
 * envelope.c
 *
 * Created: 13/04/19 18:31:53
 *  Author: Sondre
 */ 

#include "envelope.h"
#include "keyboard.h"


static uint8_t envelope_value;

uint8_t rise_speed;
uint8_t fall_speed;


void envelope_init(void)
{
	envelope_value = 0;
	rise_speed = 3;
	fall_speed = 3;
}


void envelope_update(void)
{
	volatile uint8_t gate = keyboard_get_gate();
	
	if (gate && envelope_value < 255) {
		envelope_value += rise_speed;
	}
	else if (envelope_value > 0) {
		envelope_value -= fall_speed;
	}
}


uint8_t envelope_get_value(void)
{
	return envelope_value;
}

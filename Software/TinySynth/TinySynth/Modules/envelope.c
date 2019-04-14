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
	rise_speed = 128;
	fall_speed = 128;
}


void envelope_update(void)
{
	if (keyboard_get_gate()) {
		// Exponential rise to 255. Add 0xFF to ensure the value is rounded up.
		envelope_value += MULTIPLY_UINT8_ROUND_UP(255 - envelope_value, rise_speed);
		//envelope_value += ((uint16_t)(255 - envelope_value) * ((uint16_t)rise_speed + 1) + 0xFF) >> 8;
	}
	else {
		// Exponential decay to 0. No addition before scaling to ensure the value is rounded down.
		envelope_value -= MULTIPLY_UINT8_ROUND_UP(envelope_value, fall_speed);
		//envelope_value -= ((uint16_t)envelope_value * ((uint16_t)fall_speed + 1)) >> 8;
	}
}


uint8_t envelope_get_value(void)
{
	return envelope_value;
}

/*
 * envelope.c
 *
 * Created: 13/04/19 18:31:53
 *  Author: Sondre
 */ 

#include "envelope.h"
#include "keyboard.h"

static envelope_t _envelope_a;
static envelope_t _envelope_b;

envelope_t* const ENVELOPE_A = &_envelope_a;
envelope_t* const ENVELOPE_B = &_envelope_b;


void envelope_init(envelope_t* envelope)
{
	envelope->value = 0;
	envelope->state = EGSTATE_RELEASE;
	envelope->attack_speed = 1;
	envelope->decay_speed = 1;
	envelope->sustain_value = 1;
	envelope->release_speed = 1;
}


void envelope_update(envelope_t* envelope)
{
	volatile uint8_t gate = keyboard_get_gate();
	
	if (!gate) {
		envelope->state = EGSTATE_RELEASE;
	}
	
	if (gate && envelope->state == EGSTATE_RELEASE) {
		envelope->state = EGSTATE_ATTACK;
	}
	
	switch (envelope->state)
	{
	case EGSTATE_ATTACK:
		if (envelope->value <= 255 - envelope->attack_speed) {
			envelope->value += envelope->attack_speed;
		}
		else {
			envelope->value = 255;
			envelope->state = EGSTATE_DECAY;
		}
		break;
	case EGSTATE_DECAY:
		if (envelope->value >= envelope->sustain_value + envelope->decay_speed) {
			envelope->value -= envelope->decay_speed;
		}
		else {
			envelope->value = envelope->sustain_value;
			envelope->state = EGSTATE_SUSTAIN;
		}
		break;
	case EGSTATE_SUSTAIN:
		envelope->value = envelope->sustain_value;
		break;
	case EGSTATE_RELEASE:
		if (envelope->value >= envelope->release_speed) {
			envelope->value -= envelope->release_speed;
		}
		else {
			envelope->value = 0;
		}
		break;
	}
}

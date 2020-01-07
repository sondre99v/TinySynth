/*
 * envelope.c
 *
 * Created: 12/08/19 22:04:41
 *  Author: Sondre
 */ 

#include "envelope.h"
#include <stdlib.h>

#define EG_MAX 255
#define EG_MIN 0

static envelope_t ENVELOPE_DATA_1 = {0};
static envelope_t ENVELOPE_DATA_2 = {0};
static envelope_t ENVELOPE_DATA_3 = {0};

envelope_t* const ENVELOPE_1 = &ENVELOPE_DATA_1;
envelope_t* const ENVELOPE_2 = &ENVELOPE_DATA_2;
envelope_t* const ENVELOPE_3 = &ENVELOPE_DATA_3;


void envelope_init(envelope_t* envelope)
{
	envelope->gate_source = NULL;
	envelope->trigger_source = NULL;
	envelope->attack_speed = 255;
	envelope->release_speed = 255;
	envelope->value = EG_MIN;
	envelope->inverted = false;
	envelope->reset_on_trigger = false;
}

void envelope_update(envelope_t* envelope)
{
	volatile uint8_t gate = (envelope->gate_source) == NULL ? 0 : *(envelope->gate_source);
	volatile uint8_t trigger = (envelope->trigger_source) == NULL ? 0 : *(envelope->trigger_source);

	// If inverted, flip value before processing, 
	// so that inverted and non-inverted EGs can share code
	if (envelope->inverted) {
		envelope->value = EG_MAX - envelope->value;
	}
	
	if (envelope->reset_on_trigger && trigger) {
		envelope->value = EG_MIN;
	}

	if (gate) {
		if (envelope->value < EG_MAX - envelope->attack_speed) {
			envelope->value += envelope->attack_speed;
		}
		else {
			envelope->value = EG_MAX;
		}
	}
	else {
		if (envelope->value > EG_MIN + envelope->release_speed) {
			envelope->value -= envelope->release_speed;
		}
		else {
			envelope->value = EG_MIN;
		}
	}
	
	
	if (envelope->inverted) {
		envelope->value = EG_MAX - envelope->value;
	}
}

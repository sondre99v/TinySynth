/*
 * envelope.c
 *
 * Created: 12/08/19 22:04:41
 *  Author: Sondre
 */ 

#include "envelope.h"
#include <stdlib.h>

typedef struct {
	envelope_t public;
	uint8_t hold_timer;
	enum {
		EGSTATE_ATTACK,
		EGSTATE_HOLD,
		EGSTATE_DECAY,
		EGSTATE_SUSTAIN,
		EGSTATE_RELEASE
	} state;
} envelope_data_t;


static envelope_data_t ENVELOPE_DATA_1 = {0};
static envelope_data_t ENVELOPE_DATA_2 = {0};
static envelope_data_t ENVELOPE_DATA_3 = {0};
static envelope_data_t ENVELOPE_DATA_4 = {0};

envelope_t* const ENVELOPE_1 = (envelope_t*)&ENVELOPE_DATA_1;
envelope_t* const ENVELOPE_2 = (envelope_t*)&ENVELOPE_DATA_2;
envelope_t* const ENVELOPE_3 = (envelope_t*)&ENVELOPE_DATA_3;
envelope_t* const ENVELOPE_4 = (envelope_t*)&ENVELOPE_DATA_4;


void envelope_init(envelope_t* envelope)
{
	envelope_data_t* eg_data = (envelope_data_t*)envelope;
	eg_data->public.gate_source = NULL;
	eg_data->public.attack_speed = 255;
	eg_data->public.hold_time = 0;
	eg_data->public.decay_speed = 255;
	eg_data->public.sustain_value = 255;
	eg_data->public.release_speed = 255;
	eg_data->public.value = 0;
	eg_data->hold_timer = 0;
	eg_data->state = EGSTATE_RELEASE;
}


void envelope_update(envelope_t* envelope)
{
	envelope_data_t* eg_data = (envelope_data_t*)envelope;

	volatile uint8_t gate = (eg_data->public.gate_source) == NULL ? 0 : *(eg_data->public.gate_source);

	if (!gate) {
		eg_data->state = EGSTATE_RELEASE;
	}

	if (gate && eg_data->state == EGSTATE_RELEASE) {
		eg_data->state = EGSTATE_ATTACK;
		if (eg_data->public.release_speed == 0) {
			eg_data->public.value = 0;
		}
	}

	switch (eg_data->state)
	{
		case EGSTATE_ATTACK:
			if (eg_data->public.value <= 255 - eg_data->public.attack_speed) {
				eg_data->public.value += eg_data->public.attack_speed;
			}
			else {
				eg_data->public.value = 255;
				eg_data->state = EGSTATE_HOLD;
				eg_data->hold_timer = eg_data->public.hold_time;
			}
			break;
		case EGSTATE_HOLD:
			if (eg_data->hold_timer == 0) {
				eg_data->public.value = 255;
				eg_data->state = EGSTATE_DECAY;
			} 
			else {
				eg_data->hold_timer--;
			}
			break;
		case EGSTATE_DECAY:
			if (eg_data->public.value >= eg_data->public.sustain_value + eg_data->public.decay_speed) {
				eg_data->public.value -= eg_data->public.decay_speed;
			}
			else {
				eg_data->public.value = eg_data->public.sustain_value;
				eg_data->state = EGSTATE_SUSTAIN;
			}
			break;
		case EGSTATE_SUSTAIN:
			eg_data->public.value = eg_data->public.sustain_value;
			break;
		case EGSTATE_RELEASE:
			if (eg_data->public.value >= eg_data->public.release_speed) {
				eg_data->public.value -= eg_data->public.release_speed;
			}
			else {
				eg_data->public.value = 0;
			}
			break;
	}
}

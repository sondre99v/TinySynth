/*
 * envelope.h
 *
 * Created: 13/04/19 12:17:58
 *  Author: Sondre
 */


#ifndef ENVELOPE_H_
#define ENVELOPE_H_

#include <stdint.h>

typedef struct {
	uint8_t value;
	uint8_t attack_speed;
	uint8_t decay_speed;
	uint8_t sustain_value;
	uint8_t release_speed;
	enum {
		EGSTATE_ATTACK,
		EGSTATE_DECAY,
		EGSTATE_SUSTAIN,
		EGSTATE_RELEASE
	} state;
} envelope_t;

extern envelope_t* const ENVELOPE_A;
extern envelope_t* const ENVELOPE_B;

void envelope_init(envelope_t* envelope);
void envelope_update(envelope_t* envelope);

#endif /* ENVELOPE_H_ */
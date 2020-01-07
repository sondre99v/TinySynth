/*
 * envelope.h
 *
 * Created: 12/08/19 22:02:02
 *  Author: Sondre
 */ 


#ifndef ENVELOPE_H_
#define ENVELOPE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t* gate_source;
	uint8_t* trigger_source;
	uint8_t attack_speed;
	uint8_t release_speed;
	bool inverted;
	bool reset_on_trigger;
	uint8_t value;
} envelope_t;


extern envelope_t* const ENVELOPE_1;
extern envelope_t* const ENVELOPE_2;
extern envelope_t* const ENVELOPE_3;


void envelope_init(envelope_t* envelope);
void envelope_update(envelope_t* envelope);


#endif /* ENVELOPE_H_ */
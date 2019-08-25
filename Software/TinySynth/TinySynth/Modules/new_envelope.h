/*
 * new_envelope.h
 *
 * Created: 12/08/19 22:02:02
 *  Author: Sondre
 */ 


#ifndef NEW_ENVELOPE_H_
#define NEW_ENVELOPE_H_

#include <stdint.h>

typedef struct {
	uint8_t* gate_source;
	uint8_t attack_speed;
	uint8_t hold_time;
	uint8_t decay_speed;
	uint8_t sustain_value;
	uint8_t release_speed;
	uint8_t value;
} envelope_t;


extern envelope_t* const ENVELOPE_1;
extern envelope_t* const ENVELOPE_2;
extern envelope_t* const ENVELOPE_3;
extern envelope_t* const ENVELOPE_4;


void envelope_init(envelope_t* envelope);
void envelope_update(envelope_t* envelope);
uint8_t envelope_get_value();


#endif /* NEW_ENVELOPE_H_ */
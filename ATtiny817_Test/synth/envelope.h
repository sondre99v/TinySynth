/*
 * envelope.h
 *
 * Created: 2019-02-23 01:34:09
 *  Author: Sondre
 */ 


#ifndef ENVELOPE_H_
#define ENVELOPE_H_


typedef struct {
	uint16_t attack_ms;
	uint16_t decay_ms;
	uint8_t sustain;
	uint16_t release_ms;
	uint8_t current_value;
} envelope_t;

void envelope_update(uint16_t time_delta_us);

#endif /* ENVELOPE_H_ */
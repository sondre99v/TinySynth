/*
 * envelope.h
 *
 * Created: 13/04/19 12:17:58
 *  Author: Sondre
 */ 


#ifndef ENVELOPE_H_
#define ENVELOPE_H_

// Envelope parameters
extern uint8_t rise_speed;
extern uint8_t fall_speed;


void envelope_init(void);
void envelope_update(void);

uint8_t envelope_get_value(void);


#endif /* ENVELOPE_H_ */
/*
 * keyboard.h
 *
 * Created: 13/04/19 12:13:33
 *  Author: Sondre
 */


#ifndef KEYBOARD_H_
#define KEYBOARD_H_


#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint8_t note_value;
	int8_t bend_value;
	uint8_t gate_value;
	uint8_t trigger_value;
} keyboard_t;


extern keyboard_t* const KEYBOARD_1;

void keyboard_init(keyboard_t* keyboard);
void keyboard_update(keyboard_t* keyboard);

void keyboard_pulse_gate(keyboard_t* keyboard);

void keyboard_enable_slide(keyboard_t* keyboard);
void keyboard_disable_slide(keyboard_t* keyboard);


#endif /* KEYBOARD_H_ */
/*
 * keyboard.h
 *
 * Created: 13/04/19 12:13:33
 *  Author: Sondre
 */ 


#ifndef KEYBOARD_H_
#define KEYBOARD_H_


#include <stdint.h>


void keyboard_init(void);
void keyboard_update(void);

uint8_t keyboard_get_note(void);
uint8_t keyboard_get_gate(void);


#endif /* KEYBOARD_H_ */
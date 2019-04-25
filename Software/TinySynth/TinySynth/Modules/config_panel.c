/*
 * config_panel.c
 *
 * Created: 2019-04-26 00:24:15
 *  Author: Sondre
 */ 

#include <avr/io.h>
#include "config_panel.h"

typedef enum {
	BUTTON_NONE = -1,
	BUTTON_OSC1_OCT,
	BUTTON_OSC1_WAVE,
	BUTTON_EG_RISE,
	BUTTON_EG_FALL,
	BUTTON_OSC2_OCT,
	BUTTON_OSC2_WAVE,
	BUTTON_OSC2_TUNE,
	BUTTON_SYNC
} button_t;

static button_t held_button;

void config_panel_init(void)
{
	PORTB.DIR = 0x00;
	PORTB.PIN0CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN3CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN5CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN6CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN7CTRL = PORT_PULLUPEN_bm;
}

void config_panel_update(void)
{
	uint8_t buttons = ~PORTB.IN;
	
	volatile int8_t button_index = -1;
	while(buttons != 0) {
		button_index++;
		buttons <<= 1;
	}
	
	return;
}

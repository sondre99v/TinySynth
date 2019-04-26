/*
 * patch_panel.c
 *
 * Created: 2019-04-26 00:24:15
 *  Author: Sondre
 */ 
 
#include "patch_panel.h"

#include <avr/io.h>

#include "patch.h"
#include "keyboard.h"

typedef enum {
	BUTTON_NONE = -1,
	BUTTON_OSC1_OCT = 7,
	BUTTON_OSC1_WAVE = 6,
	BUTTON_EG_RISE = 5,
	BUTTON_EG_FALL = 4,
	BUTTON_OSC2_OCT = 3,
	BUTTON_OSC2_WAVE = 2,
	BUTTON_OSC2_TUNE = 1,
	BUTTON_SYNC = 0
} button_t;

static button_t held_button;
static uint8_t button_holdoff_timer = 0;

void _set_led(uint8_t led, bool on) {
	if (led < 6) {
		if (on) PORTC.OUTCLR = (1 << led);
		else    PORTC.OUTSET = (1 << led);
	}
	else {
		if (on) PORTA.OUTCLR = (1 << (led - 5));
		else    PORTA.OUTSET = (1 << (led - 5));
	}
}

void patch_panel_init(void)
{
	// Enable the buttons
	PORTB.DIR = 0x00;
	PORTB.PIN0CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN1CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN2CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN3CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN4CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN5CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN6CTRL = PORT_PULLUPEN_bm;
	PORTB.PIN7CTRL = PORT_PULLUPEN_bm;
	
	// Enable the LEDs
	PORTC.DIRSET = 0x3F;
	PORTA.DIRSET = 0x06;
	PORTC.OUTSET = 0x3F;
	PORTA.OUTSET = 0x06;
}

void patch_panel_update(void)
{
	if (button_holdoff_timer) {
		button_holdoff_timer--;
		return;
	}
	
	uint8_t buttons = ~PORTB.IN;
	
	volatile button_t button_event = BUTTON_NONE;
	if (buttons == 0x00) {
		held_button = BUTTON_NONE;
	}

	for (int i = 0; i < 8; i++) {
		if (buttons & (1 << i)) {
			if (held_button != (button_t)i) {
				button_event = (button_t)i;
			}
			held_button = (button_t)i;

			break;
		}
	}

	switch (button_event)
	{
		case BUTTON_OSC1_OCT:
			patch_cycle_oscA_octave();
			break;
		case BUTTON_OSC1_WAVE:
			patch_cycle_oscA_wave();
			break;
		case BUTTON_EG_RISE:
			patch_toggle_eg_rise();
			break;
		case BUTTON_EG_FALL:
			patch_toggle_eg_fall();
			break;
		case BUTTON_OSC2_OCT:
			patch_cycle_oscB_octave();
			break;
		case BUTTON_OSC2_WAVE:
			patch_cycle_oscB_wave();
			break;
		case BUTTON_OSC2_TUNE:
			patch_cycle_oscB_detune();
			break;
		case BUTTON_SYNC:
			patch_toggle_sync();
			break;
		default: break;
	}
	
	if (button_event != BUTTON_NONE) {
		keyboard_pulse_gate();
		button_holdoff_timer = 50;
	}
}

void patch_panel_set_led(patch_led_t led, uint8_t value)
{
	switch(led) {
		case PATCH_LED_OSCA_WAVE:
			_set_led(6, value & 0x1);
			_set_led(7, value & 0x2);
			break;
		case PATCH_LED_OSCB_ENABLED:
			_set_led(3, value & 0x1);
			break;
		case PATCH_LED_OSCB_WAVE:
			_set_led(1, value & 0x1);
			_set_led(2, value & 0x2);
			break;
		case PATCH_LED_SYNC:
			_set_led(0, value & 0x1);
			break;
		case PATCH_LED_EG_RISE:
			_set_led(5, value & 0x1);
			break;
		case PATCH_LED_EG_FALL:
			_set_led(4, value & 0x1);
			break;
		default:
			break;
	}
}

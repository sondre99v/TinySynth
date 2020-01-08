/*
 * TinySynth.c
 *
 * Created: 12/04/19 17:22:15
 * Author : Sondre
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#include "Modules/oscillator.h"
#include "Modules/keyboard.h"
#include "Modules/envelope.h"
#include "Modules/lfo.h"
#include "Modules/patch_panel.h"
#include "Modules/patch.h"

#define TIME_TIMER_PERIOD 6250 // Gives 100Hz frequency with 20MHz clock and 32x prescaler

volatile static uint8_t update_pending = 0;

ISR(TCD0_OVF_vect) {
	TCD0.INTFLAGS = 0x01;
	update_pending = 1;
}

extern uint8_t gate_value;

int main(void)
{
	// Disable main clock prescaler to get 10MHz speed
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 1;

	int8_t bend_sum = 0;

	// Setup TCD0 to give a 100Hz interrupt
	TCD0.CMPBCLR = TIME_TIMER_PERIOD;
	TCD0.INTCTRL = TCD_OVF_bm;
	TCD0.CTRLA = TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;

	oscillator_init();
	oscillator_set_sources(OSCILLATOR_A, &(KEYBOARD_1->note_value), &(KEYBOARD_1->bend_value), &(LFO_1->value), &(ENVELOPE_1->value));
	oscillator_set_sources(OSCILLATOR_B, &(KEYBOARD_1->note_value), &(KEYBOARD_1->bend_value), &(LFO_1->value), &(ENVELOPE_2->value));
	
	keyboard_init(KEYBOARD_1);
	envelope_init(ENVELOPE_1);
	envelope_init(ENVELOPE_2);
	//envelope_init(ENVELOPE_3);
	lfo_init(LFO_1);
	ENVELOPE_1->gate_source = &(KEYBOARD_1->gate_value);
	ENVELOPE_1->trigger_source = &(KEYBOARD_1->trigger_value);
	ENVELOPE_2->gate_source = &(KEYBOARD_1->gate_value);
	ENVELOPE_2->trigger_source = &(KEYBOARD_1->trigger_value);
	ENVELOPE_3->gate_source = &(KEYBOARD_1->gate_value);
	ENVELOPE_3->trigger_source = &(KEYBOARD_1->trigger_value);
	
	patch_panel_init();
	patch_init();
	
	sei();

	while (1)
    {
		if (update_pending) {
			patch_panel_update();
			keyboard_update(KEYBOARD_1);
			lfo_update(LFO_1);
			bend_sum = KEYBOARD_1->bend_value + LFO_1->value;
			oscillator_update(OSCILLATOR_A);
			oscillator_update(OSCILLATOR_B);
			envelope_update(ENVELOPE_1);
			envelope_update(ENVELOPE_2);
			envelope_update(ENVELOPE_3);
			update_pending = 0;
		}
    }
}


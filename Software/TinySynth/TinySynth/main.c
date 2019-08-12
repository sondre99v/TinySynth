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
#include "Modules/new_envelope.h"
#include "Modules/patch_panel.h"
#include "Modules/patch.h"

#define TIME_TIMER_PERIOD 6250 // Gives 100Hz frequency with 20MHz clock and 32x prescaler

volatile static uint8_t update_pending = 0;
uint8_t led_pwm_counter = 0;

ISR(TCD0_OVF_vect) {
	TCD0.INTFLAGS = 0x01;
	update_pending = 1;

	led_pwm_counter = (led_pwm_counter + 1) % 4;

	if (led_pwm_counter == 0) {
		PORTC.DIRSET = 0xFE;
		PORTA.DIRSET = 0x06;
	}
	else {
		PORTC.DIRCLR = 0xFE;
		PORTA.DIRCLR = 0x06;
	}
}

extern uint8_t gate_value;

int main(void)
{
	// Disable main clock prescaler to get 10MHz speed
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 1;



	// Setup TCD0 to give a 100Hz interrupt
	TCD0.CMPBCLR = TIME_TIMER_PERIOD;
	TCD0.INTCTRL = TCD_OVF_bm;
	TCD0.CTRLA = TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;

	oscillator_init();
	oscillator_set_sources(OSCILLATOR_A, &keyboard_note, &(ENVELOPE_1->value));
	oscillator_set_sources(OSCILLATOR_B, &keyboard_note, &(ENVELOPE_2->value));
	
	keyboard_init();
	envelope_init(ENVELOPE_1);
	envelope_init(ENVELOPE_2);
	envelope_init(ENVELOPE_3);
	ENVELOPE_1->gate_source = &gate_value;
	ENVELOPE_2->gate_source = &gate_value;
	ENVELOPE_3->gate_source = &ENVELOPE_2->value;
	ENVELOPE_3->attack_speed = 0xFF;
	ENVELOPE_3->hold_time = 0;
	ENVELOPE_3->decay_speed = 0x3;
	ENVELOPE_3->sustain_value = 0x40;
	ENVELOPE_3->release_speed = 0;
	patch_panel_init();
	patch_init();

	
	oscillator_set_waveform(OSCILLATOR_A, WAVE_SAW);
	oscillator_set_waveform(OSCILLATOR_B, WAVE_SILENCE);
	

	sei();

	while (1)
    {
		if (update_pending) {
			patch_panel_update();
			keyboard_update();
			oscillator_update(OSCILLATOR_A);
			oscillator_update(OSCILLATOR_B);
			envelope_update(ENVELOPE_1);
			envelope_update(ENVELOPE_2);
			envelope_update(ENVELOPE_3);
			update_pending = 0;
		}
    }
}


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
#include "Modules/patch_panel.h"
#include "Modules/patch.h"

#define TIME_TIMER_PERIOD 6250 // Gives 100Hz frequency with 20MHz clock and 32x prescaler

volatile static uint8_t update_pending = 0;

ISR(TCD0_OVF_vect) {
	TCD0.INTFLAGS = 0x01;
	update_pending = 1;
}

int main(void)
{
	// Disable main clock prescaler to get 10MHz speed
	CCP = CCP_IOREG_gc;
	CLKCTRL.MCLKCTRLB = 1;

	uint16_t freqs[] = {
		1746, 1850, 1960, 2077, 2200, 2331, 2469, 2616, 2772, 2937,
		3111, 3296, 3492, 3700, 3920, 4153, 4400, 4662, 4939, 5233,
	};
	
	// Setup TCD0 to give a 100Hz interrupt
	TCD0.CMPBCLR = TIME_TIMER_PERIOD;
	TCD0.INTCTRL = TCD_OVF_bm;
	TCD0.CTRLA = TCD_CNTPRES_DIV32_gc | TCD_ENABLE_bm;

	oscillator_init();
	keyboard_init();
	envelope_init();
	patch_panel_init();
	patch_init();

	sei();

	while (1) 
    {
		if (update_pending) {
			patch_panel_update();
			keyboard_update();
			envelope_update();
		
			volatile uint8_t amp_eg = envelope_get_value();
		
			oscillator_set_amplitude(OSCILLATOR_A, amp_eg >> 2);
			oscillator_set_amplitude(OSCILLATOR_B, amp_eg >> 2);
			oscillator_set_frequency(OSCILLATOR_A, freqs[keyboard_get_note()]);
			oscillator_set_frequency(OSCILLATOR_B, freqs[keyboard_get_note()]);//+freqs[keyboard_get_note()]/2);
			update_pending = 0;
		}

    }
}


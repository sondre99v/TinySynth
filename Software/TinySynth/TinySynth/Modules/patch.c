/*
 * patch.c
 *
 * Created: 26/04/19 16:08:57
 *  Author: Sondre
 */

#include "patch.h"

#include <stdbool.h>
#include <stdint.h>

#include "envelope.h"
#include "keyboard.h"
#include "patch_panel.h"

#define EG_FAST_RISE_SPEED 255
#define EG_SLOW_RISE_SPEED 2
#define EG_FAST_FALL_SPEED 50
#define EG_SLOW_FALL_SPEED 2

static const patch_t default_patch = {
	.oscA_note_offset = 0,
	.oscA_wave = WAVE_SQUARE,
	.oscB_note_offset = 0,
	.oscB_wave = WAVE_SAW,
	.oscA_enabled = true,
	.oscB_enabled = false,
	.slide_enabled = false,
	.eg_rise_speed = EG_FAST_RISE_SPEED,
	.eg_fall_speed = EG_SLOW_FALL_SPEED
};

static const patch_t debug_patchA = {
	.oscA_note_offset = 0,
	.oscA_wave = WAVE_SQUARE,
	.oscB_note_offset = 0,
	.oscB_wave = WAVE_SQUARE,
	.oscA_enabled = true,
	.oscB_enabled = true,
	.slide_enabled = false,
	.eg_rise_speed = EG_FAST_RISE_SPEED,
	.eg_fall_speed = EG_FAST_FALL_SPEED
};


static patch_t active_patch;

void _apply_patch(const patch_t* patch)
{
	active_patch = *patch;

	oscillator_set_note_offset(OSCILLATOR_A, active_patch.oscA_note_offset);
	oscillator_set_waveform(OSCILLATOR_A, active_patch.oscA_enabled ? active_patch.oscA_wave : WAVE_SILENCE);
	oscillator_set_filter_mod_amount(OSCILLATOR_A, 0);
	oscillator_set_pitch_mod_amount(OSCILLATOR_A, 0);

	oscillator_set_note_offset(OSCILLATOR_B, active_patch.oscB_note_offset);
	oscillator_set_waveform(OSCILLATOR_B, active_patch.oscB_enabled ? active_patch.oscB_wave : WAVE_SILENCE);
	oscillator_set_filter_mod_amount(OSCILLATOR_B, 0);
	oscillator_set_pitch_mod_amount(OSCILLATOR_B, 0);

	oscillator_set_percussive(false);

	active_patch.slide_enabled ? keyboard_enable_slide(KEYBOARD_1) : keyboard_disable_slide(KEYBOARD_1);

	ENVELOPE_1->attack_speed = active_patch.eg_rise_speed;
	ENVELOPE_1->release_speed = active_patch.eg_fall_speed;

	ENVELOPE_2->attack_speed = active_patch.eg_rise_speed;
	ENVELOPE_2->release_speed = active_patch.eg_fall_speed;

	ENVELOPE_3->reset_on_trigger = 1;
	ENVELOPE_3->release_on_trigger = 0;

	switch(active_patch.effect) {
		case EFFECT_FILTER:
			ENVELOPE_3->attack_speed = 1;
			ENVELOPE_3->release_speed = 0;
			oscillator_set_filter_mod_amount(OSCILLATOR_A, 96);
			oscillator_set_filter_mod_amount(OSCILLATOR_B, 96);
			break;
		case EFFECT_HIT:
			ENVELOPE_3->attack_speed = 255;
			ENVELOPE_3->release_speed = 10;
			ENVELOPE_3->release_on_trigger = 1;
			oscillator_set_percussive(true);
			break;
		case EFFECT_SHAKE:
			oscillator_set_pitch_mod_amount(OSCILLATOR_A, 64);
			oscillator_set_pitch_mod_amount(OSCILLATOR_B, 64);
			break;
		default: break;
	}

	patch_panel_set_led(PATCH_LED_OSCA_WAVE, active_patch.oscA_wave == WAVE_SQUARE ? 1 : 0);
	patch_panel_set_led(PATCH_LED_OSCB_ENABLED, (uint8_t)active_patch.oscB_enabled);
	patch_panel_set_led(PATCH_LED_OSCB_WAVE, active_patch.oscB_wave == WAVE_SQUARE ? 1 : 0);
	patch_panel_set_led(PATCH_LED_SLIDE, (uint8_t)active_patch.slide_enabled);
	patch_panel_set_led(PATCH_LED_EG_RISE, active_patch.eg_rise_speed < EG_FAST_RISE_SPEED ? 1 : 0);
	patch_panel_set_led(PATCH_LED_EG_FALL, active_patch.eg_fall_speed < EG_FAST_FALL_SPEED ? 1 : 0);
	patch_panel_set_led(PATCH_LED_EFFECT, active_patch.effect);
}

void patch_init(void)
{
	_apply_patch(&default_patch);
}

void patch_cycle_oscA_pitch(void)
{
	switch(active_patch.oscA_note_offset) {
		case 0:  active_patch.oscA_note_offset = 12; break;
		case 12: active_patch.oscA_note_offset = 24; break;
		default: active_patch.oscA_note_offset = 0; break;
	}

	_apply_patch(&active_patch);
}

void patch_cycle_oscA_wave(void)
{
	if (active_patch.oscA_wave == WAVE_SQUARE) {
		active_patch.oscA_wave = WAVE_SAW;
	} 
	else {
		active_patch.oscA_wave = WAVE_SQUARE;
	}

	_apply_patch(&active_patch);
}

void patch_cycle_oscB_pitch(void)
{
	if (!active_patch.oscB_enabled) {
		active_patch.oscB_enabled = true;
		active_patch.oscB_note_offset = 0;
	}
	else {
		switch(active_patch.oscB_note_offset) {
			case 0:  active_patch.oscB_note_offset = 4; break;
			case 4:  active_patch.oscB_note_offset = 7; break;
			case 7:  active_patch.oscB_note_offset = 12; break;
			case 12: active_patch.oscB_note_offset = 24; break;
			default: active_patch.oscB_enabled = false; break;
		}
	}

	_apply_patch(&active_patch);
}

void patch_cycle_oscB_wave(void)
{
	if (active_patch.oscB_wave == WAVE_SQUARE) {
		active_patch.oscB_wave = WAVE_SAW;
	}
	else {
		active_patch.oscB_wave = WAVE_SQUARE;
	}

	_apply_patch(&active_patch);
}

void patch_toggle_eg_rise(void)
{
	if (active_patch.eg_rise_speed < EG_FAST_RISE_SPEED) {
		active_patch.eg_rise_speed = EG_FAST_RISE_SPEED;
	}
	else {
		active_patch.eg_rise_speed = EG_SLOW_RISE_SPEED;
	}
	_apply_patch(&active_patch);
}

void patch_toggle_eg_fall(void)
{
	if (active_patch.eg_fall_speed < EG_FAST_FALL_SPEED) {
		active_patch.eg_fall_speed = EG_FAST_FALL_SPEED;
	}
	else {
		active_patch.eg_fall_speed = EG_SLOW_FALL_SPEED;
	}
	_apply_patch(&active_patch);
}

void patch_toggle_slide(void)
{
	active_patch.slide_enabled = !active_patch.slide_enabled;
	_apply_patch(&active_patch);
}

void patch_toggle_effect(void)
{
	active_patch.effect = (effect_t)(((int)active_patch.effect + 1) % 4);
	_apply_patch(&active_patch);
}

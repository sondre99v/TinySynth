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

#define GLIDE_ENABLED_SPEED 10
#define GLIDE_DISABLED_SPEED 65535

//   0 - Unison
//  43 - Minor third
//  51 - Major third
//  64 - Major fourth
//  85 - Fifth
// 128 - Octave
#define DETUNE_STEP_0 0
#define DETUNE_STEP_1 43
#define DETUNE_STEP_2 51
#define DETUNE_STEP_3 64
#define DETUNE_STEP_4 85


static const patch_t default_patch = {
	.oscA_octave_offset = 0,
	.oscA_wave = WAVE_SAW,
	.oscB_octave_offset = 1,
	.oscB_wave = WAVE_SQUARE,
	.oscB_detune = 128,
	.oscA_enabled = true,
	.oscB_enabled = false,
	.slide = false,
	.eg_rise_speed = EG_FAST_RISE_SPEED,
	.eg_fall_speed = EG_SLOW_FALL_SPEED
};

static const patch_t debug_patchA = {
	.oscA_octave_offset = 0,
	.oscA_wave = WAVE_SQUARE,
	.oscB_octave_offset = 0,
	.oscB_wave = WAVE_SQUARE,
	.oscB_detune = DETUNE_STEP_0,
	.oscA_enabled = true,
	.oscB_enabled = true,
	.slide = false,
	.eg_rise_speed = EG_FAST_RISE_SPEED,
	.eg_fall_speed = EG_FAST_FALL_SPEED
};


static patch_t active_patch;

void _apply_patch(const patch_t* patch)
{
	active_patch = *patch;

	oscillator_set_octave(OSCILLATOR_A, active_patch.oscA_octave_offset);
	oscillator_set_waveform(OSCILLATOR_A, active_patch.oscA_enabled ? active_patch.oscA_wave : WAVE_SILENCE);

	oscillator_set_octave(OSCILLATOR_B, active_patch.oscB_octave_offset);
	oscillator_set_waveform(OSCILLATOR_B, active_patch.oscB_enabled ? active_patch.oscB_wave : WAVE_SILENCE);

	active_patch.slide ? keyboard_enable_slide(KEYBOARD_1) : keyboard_disable_slide(KEYBOARD_1);
	
	ENVELOPE_1->attack_speed = active_patch.eg_rise_speed;
	ENVELOPE_1->hold_time = 0;
	ENVELOPE_1->decay_speed = 255;
	ENVELOPE_1->sustain_value = 255;
	ENVELOPE_1->release_speed = active_patch.eg_fall_speed;
	
	ENVELOPE_2->attack_speed = active_patch.eg_rise_speed;
	ENVELOPE_2->hold_time = 0;
	ENVELOPE_2->decay_speed = 255;
	ENVELOPE_2->sustain_value = 255;
	ENVELOPE_2->release_speed = active_patch.eg_fall_speed;

	patch_panel_set_led(PATCH_LED_OSCA_WAVE, active_patch.oscA_wave == WAVE_SQUARE ? 1 : 0);
	patch_panel_set_led(PATCH_LED_OSCB_ENABLED, (uint8_t)active_patch.oscB_enabled);
	patch_panel_set_led(PATCH_LED_OSCB_WAVE, active_patch.oscB_wave == WAVE_SQUARE ? 1 : 0);
	patch_panel_set_led(PATCH_LED_SLIDE, (uint8_t)active_patch.slide);
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
	if (!active_patch.oscA_enabled) {
		active_patch.oscA_enabled = true;
		active_patch.oscA_octave_offset = 0;
	}
	else {
		active_patch.oscA_octave_offset++;
		if (active_patch.oscA_octave_offset > 2) {
			active_patch.oscA_enabled = false;
		}
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
		active_patch.oscB_octave_offset = 0;
	}
	else {
		active_patch.oscB_octave_offset++;
		if (active_patch.oscB_octave_offset > 2) {
			active_patch.oscB_enabled = false;
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
	active_patch.slide = !active_patch.slide;
	_apply_patch(&active_patch);
}

void patch_toggle_effect(void)
{
	active_patch.effect = (effect_t)(((int)active_patch.effect + 1) % 4);
	_apply_patch(&active_patch);
}

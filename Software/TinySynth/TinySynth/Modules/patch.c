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
#include "patch_panel.h"

#define EG_FAST_RISE_SPEED 255
#define EG_SLOW_RISE_SPEED 2
#define EG_FAST_FALL_SPEED 50
#define EG_SLOW_FALL_SPEED 2

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
	.oscA_octave = 0,
	.oscA_wave = WAVE_SAW,
	.oscB_octave = 1,
	.oscB_wave = WAVE_SQUARE,
	.oscB_detune = 128,
	.oscB_enabled = false,
	.sync = false,
	.eg_rise_speed = EG_FAST_RISE_SPEED,
	.eg_fall_speed = EG_SLOW_FALL_SPEED
};

static const patch_t debug_patchA = {
	.oscA_octave = 0,
	.oscA_wave = WAVE_SQUARE,
	.oscB_octave = 0,
	.oscB_wave = WAVE_SQUARE,
	.oscB_detune = DETUNE_STEP_0,
	.oscB_enabled = true,
	.sync = false,
	.eg_rise_speed = EG_FAST_RISE_SPEED,
	.eg_fall_speed = EG_FAST_FALL_SPEED
};


static patch_t active_patch;

void _apply_patch(const patch_t* patch)
{
	active_patch = *patch;

	oscillator_set_octave(OSCILLATOR_A, active_patch.oscA_octave);
	oscillator_set_waveform(OSCILLATOR_A, active_patch.oscA_wave);

	oscillator_set_octave(OSCILLATOR_B, active_patch.oscB_octave);
	oscillator_set_waveform(OSCILLATOR_B, active_patch.oscB_enabled ? active_patch.oscB_wave : WAVE_SILENCE);

	oscillator_set_detune(OSCILLATOR_B, active_patch.oscB_detune);
	oscillator_set_sync(active_patch.sync);
	ENVELOPE_A->attack_speed = active_patch.eg_rise_speed;
	ENVELOPE_A->decay_speed = 255;
	ENVELOPE_A->sustain_value = 255;
	ENVELOPE_A->release_speed = active_patch.eg_fall_speed;
	ENVELOPE_B->attack_speed = 255;//active_patch.eg_rise_speed;
	ENVELOPE_B->decay_speed = 10;
	ENVELOPE_B->sustain_value = 0;
	ENVELOPE_B->release_speed = 10;//active_patch.eg_fall_speed;
	
	patch_panel_set_led(PATCH_LED_OSCA_WAVE, (uint8_t)active_patch.oscA_wave);
	patch_panel_set_led(PATCH_LED_OSCB_ENABLED, (uint8_t)active_patch.oscB_enabled);
	patch_panel_set_led(PATCH_LED_OSCB_WAVE, (uint8_t)active_patch.oscB_wave);
	patch_panel_set_led(PATCH_LED_SYNC, (uint8_t)active_patch.sync);
	patch_panel_set_led(PATCH_LED_EG_RISE, active_patch.eg_rise_speed < EG_FAST_RISE_SPEED ? 1 : 0);
	patch_panel_set_led(PATCH_LED_EG_FALL, active_patch.eg_fall_speed < EG_FAST_FALL_SPEED ? 1 : 0);
}

void patch_init(void)
{
	//_apply_patch(&default_patch);
	_apply_patch(&debug_patchA);
}

void patch_cycle_oscA_octave(void)
{
	active_patch.oscA_octave = (active_patch.oscA_octave + 1) % 3;
	_apply_patch(&active_patch);
}

void patch_cycle_oscA_wave(void)
{
	active_patch.oscA_wave = (waveform_t)((int)active_patch.oscA_wave + 1) % 5;
	_apply_patch(&active_patch);
}

void patch_cycle_oscB_octave(void)
{
	if (!active_patch.oscB_enabled) {
		active_patch.oscB_enabled = true;
		active_patch.oscB_octave = 0;
	} 
	else {
		active_patch.oscB_octave++;
		if (active_patch.oscB_octave > 2) {
			active_patch.oscB_enabled = false;
		}
	}
	
	_apply_patch(&active_patch);
}

void patch_cycle_oscB_wave(void)
{
	active_patch.oscB_wave = (waveform_t)((int)active_patch.oscB_wave + 1) % 5;
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

void patch_cycle_oscB_detune(void)
{
	switch(active_patch.oscB_detune) {
		case DETUNE_STEP_0: active_patch.oscB_detune = DETUNE_STEP_1; break;
		case DETUNE_STEP_1: active_patch.oscB_detune = DETUNE_STEP_2; break;
		case DETUNE_STEP_2: active_patch.oscB_detune = DETUNE_STEP_3; break;
		case DETUNE_STEP_3: active_patch.oscB_detune = DETUNE_STEP_4; break;
		case DETUNE_STEP_4: active_patch.oscB_detune = DETUNE_STEP_0; break;
		default: active_patch.oscB_detune = 0; break;
	}
	_apply_patch(&active_patch);
}

void patch_toggle_sync(void)
{
	active_patch.sync = !active_patch.sync;
	_apply_patch(&active_patch);
}

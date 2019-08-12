/*
 * patch_panel.h
 *
 * Created: 2019-04-25 23:37:52
 *  Author: Sondre
 */


#ifndef PATCH_PANEL_H_
#define PATCH_PANEL_H_

#include <stdint.h>

typedef enum {
	PATHC_LED_NONE,
	PATCH_LED_OSCA_WAVE,
	PATCH_LED_OSCB_ENABLED,
	PATCH_LED_OSCB_WAVE,
	PATCH_LED_GLIDE,
	PATCH_LED_EG_RISE,
	PATCH_LED_EG_FALL
} patch_led_t;


void patch_panel_init(void);
void patch_panel_update(void);

void patch_panel_set_led(patch_led_t led, uint8_t value);

#endif /* PATCH_PANEL_H_ */
/*
 * dac.c
 *
 * Created: 2019-02-17 08:24:09
 *  Author: Sondre
 */ 

#include "dac.h"
#include <avr/io.h>


void dac_init(DAC_reference_t reference_voltage)
{
	// Enable the chosen reference for the DAC
	VREF.CTRLA = (reference_voltage << VREF_DAC0REFSEL0_bp);
	
	// Set the output voltage to 0V
	DAC0.DATA = 0;
	
	// Enable the DAC and enable the output
	DAC0.CTRLA = DAC_OUTEN_bm | DAC_ENABLE_bm;
}

void dac_set_data(uint8_t data)
{
	DAC0.DATA = data;
}


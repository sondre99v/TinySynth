/*
 * dac.h
 *
 * Created: 2019-02-17 08:24:00
 *  Author: Sondre
 */ 


#ifndef DAC_H_
#define DAC_H_


#include <stdint.h>


typedef enum {
	REF_0_55V = 0x0,
	REF_1_1V = 0x1,
	REF_2_5V = 0x2,
	REF_4_3V = 0x3,
	REF_1_5V = 0x4
} DAC_reference_t;


void dac_init(DAC_reference_t reference_voltage);

void dac_set_data(uint8_t data);


#endif /* DAC_H_ */
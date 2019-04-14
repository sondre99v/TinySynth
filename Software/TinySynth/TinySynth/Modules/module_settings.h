/*
 * module_settings.h
 *
 * Created: 13/04/19 12:14:31
 *  Author: Sondre
 */ 


#ifndef MODULE_SETTINGS_H_
#define MODULE_SETTINGS_H_


#define MODULE_UPDATE_FREQUENCY 100 // Hz


#define MULTIPLY_UINT8(carrier, modulator) \
	((uint8_t)(((uint16_t)(carrier) * (modulator + 1) + 0x80) >> 8))
	
#define MULTIPLY_UINT8_ROUND_DOWN(carrier, modulator) \
	((uint8_t)(((uint16_t)(carrier) * (modulator + 1)) >> 8))
	
#define MULTIPLY_UINT8_ROUND_UP(carrier, modulator) \
	((uint8_t)(((uint16_t)(carrier) * (modulator + 1) + 0xFF) >> 8))

#define MULTIPLY_UINT16(carrier, modulator) \
	((uint16_t)(((uint32_t)(carrier) * (modulator + 1) + 0x8000) >> 16))
	
#define MULTIPLY_UINT16_ROUND_DOWN(carrier, modulator) \
	((uint16_t)(((uint32_t)(carrier) * (modulator + 1)) >> 16))
	
#define MULTIPLY_UINT16_ROUND_UP(carrier, modulator) \
	((uint16_t)(((uint32_t)(carrier) * (modulator + 1) + 0xFFFF) >> 16))


#endif /* MODULE_SETTINGS_H_ */
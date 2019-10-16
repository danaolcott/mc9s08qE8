/*
 * eeprom.h
 *
 *  Created on: Oct 13, 2019
 *      Author: danao
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"

void EEPROM_init(void);
void EEPROM_writeByte(uint8_t memoryAddress, uint8_t data);
uint8_t EEPROM_readByte(uint8_t memoryAddress);


#endif /* EEPROM_H_ */

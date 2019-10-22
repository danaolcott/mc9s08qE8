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

//addresses
#define EEPROM_WRITE_DELAY						((uint8_t)1)

#define EEPROM_ADDRESS_CYCLE_COUNT_MSB			((uint8_t)0x02)
#define EEPROM_ADDRESS_CYCLE_COUNT_LSB			((uint8_t)0x03)

void EEPROM_writeByte(uint8_t memoryAddress, uint8_t data);
uint8_t EEPROM_readByte(uint8_t memoryAddress);

//stats
//game cycle count.
uint16_t EEPROM_updateCycleCount(void);
uint16_t EEPROM_readCycleCount(void);

#endif /* EEPROM_H_ */

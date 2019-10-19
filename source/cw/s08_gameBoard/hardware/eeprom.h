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
#define EEPROM_ADDRESS_CHIP_ID					((uint8_t)0x00)

#define EEPROM_ADDRESS_CYCLE_COUNT_MSB			((uint8_t)0x02)
#define EEPROM_ADDRESS_CYCLE_COUNT_LSB			((uint8_t)0x03)

#define EEPROM_ADDRESS_HIGH_SCORE_MSB			((uint8_t)0x04)
#define EEPROM_ADDRESS_HIGH_SCORE_LSB			((uint8_t)0x05)



void EEPROM_init(void);
void EEPROM_writeByte(uint8_t memoryAddress, uint8_t data);
uint8_t EEPROM_readByte(uint8_t memoryAddress);

//stats
//game cycle count.
uint16_t EEPROM_updateCycleCount(void);
uint16_t EEPROM_readCycleCount(void);
uint16_t EEPROM_getCycleCount(void);

#endif /* EEPROM_H_ */

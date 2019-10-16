/*
 * eeprom.c
 *
 *  Created on: Oct 13, 2019
 *      Author: danao
 *  
 *  Memory controller file for the 24AA01 / 24LC01B.
 *  The eeprom ic uses the i2c interface and uses 
 *  address 0x50.  The general format consists of:
 *  
 *  write:
 *  i2c address, 8bit memory address, data0, data1, data2...
 *  
 *  read:
 *  i2c address, 8bit memory address,  i2c address, data read0, data read 1, etc
 *  
 *  Memory address max = 7 bits, 0x7F
 *  
 *  
 */

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"
#include "eeprom.h"
#include "i2c.h"

///////////////////////////////////////
//EEPROM_init()
void EEPROM_init(void)
{
	
}


//////////////////////////////////////////
//EEPROM_writeByte.
//Writes 2 bytes to eeprom: memory address, data
//
void EEPROM_writeByte(uint8_t memoryAddress, uint8_t data)
{
	uint8_t status = 0x00;	
	status = I2C_write2Bytes(I2C_ADDRESS, memoryAddress, data);
}

//////////////////////////////////////////
//EEPROM_readByte.
//Read data byte from memory address
uint8_t EEPROM_readByte(uint8_t memoryAddress)
{
	uint8_t result = 0x00;	
	result = I2C_memoryRead(I2C_ADDRESS, memoryAddress);
	return result;
}






/*
 * eeprom.c
 *
 *  Created on: Oct 13, 2019
 *      Author: danao
 *  
 *  Memory controller file for the 24AA01 / 24LC01B.
 *  The eeprom ic uses the i2c interface and uses 
 *  address 0x50.  The general format consists 
 *  write:
 *  i2c address, 8bit memory address, data0, data1, data2...
 *  read:
 *  i2c address, 8bit memory address,  i2c address, data read0, data read 1, etc
 *  Memory address max = 7 bits, 0x7F
 *  
 */

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"
#include "eeprom.h"
#include "i2c.h"
#include "rtc.h"



//eemprom memory locations
static unsigned int EEPROM_cycleCount @ 0x244u;



///////////////////////////////////////
//EEPROM_init()
//Load memory values into RAM, increment
//the cycle counter
void EEPROM_init(void)
{	
	EEPROM_cycleCount = 0x00;
	
	//read the high score and updated cycle count into RAM
	EEPROM_cycleCount = EEPROM_updateCycleCount();
	

}


//////////////////////////////////////////
//EEPROM_writeByte.
//Writes 2 bytes to eeprom: memory address, data
//
void EEPROM_writeByte(uint8_t memoryAddress, uint8_t data)
{
	uint8_t status = 0x00;
	RTC_delay(EEPROM_WRITE_DELAY);
	status = I2C_write2Bytes(I2C_ADDRESS, memoryAddress, data);
	RTC_delay(EEPROM_WRITE_DELAY);
}

//////////////////////////////////////////
//EEPROM_readByte.
//Read data byte from memory address
uint8_t EEPROM_readByte(uint8_t memoryAddress)
{
	uint8_t result = 0x00;
	RTC_delay(EEPROM_WRITE_DELAY);
	result = I2C_memoryRead(I2C_ADDRESS, memoryAddress);
	RTC_delay(EEPROM_WRITE_DELAY);
	return result;
}






//////////////////////////////////////////
//Reads the cycle count, increments the 
//value, then writes it back, returns the 
//new value
uint16_t EEPROM_updateCycleCount(void)
{
	uint16_t result = 0x00;
	uint8_t high = 0x00;
	uint8_t low = 0x00;
	
	result = EEPROM_readCycleCount();
	result += 1;
	
	high = (uint8_t)((result >> 8) & 0xFF);
	low = (uint8_t)(result & 0xFF);
	
	EEPROM_writeByte(EEPROM_ADDRESS_CYCLE_COUNT_MSB, high);
	EEPROM_writeByte(EEPROM_ADDRESS_CYCLE_COUNT_LSB, low);
	
	EEPROM_cycleCount = result;
	
	return result;
}


/////////////////////////////////////////////////
//Reads the current cycle count from EEPROM
//and returns the value
uint16_t EEPROM_readCycleCount(void)
{
	uint16_t low = 0x00;
	uint16_t high = 0x00;
	uint16_t result = 0x00;
	
	low = EEPROM_readByte(EEPROM_ADDRESS_CYCLE_COUNT_LSB);
	high = EEPROM_readByte(EEPROM_ADDRESS_CYCLE_COUNT_MSB);

	result = ((high & 0xFF) << 8);
	result |= (low & 0xFF);

	EEPROM_cycleCount = result;
	
	return result;
}


////////////////////////////////////////////
//Get the current cycle count from RAM
uint16_t EEPROM_getCycleCount(void)
{
	return EEPROM_cycleCount;
}

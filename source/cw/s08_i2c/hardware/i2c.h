/*
 * i2c.h
 *
 *  Created on: Sep 28, 2019
 *      Author: danao
 *  
 *  I2C Controller file for use on PA2 and PA3.  Configures
 *  the peripheral for master mode, interrupt driven.  The 
 *  approach and ISR generally follow along with the peripheral
 *  guide.  The light sensor breakout board from Adafruit is used
 *  to test the i2c.  The 7bit address is 0x39
 */

#ifndef I2C_H_
#define I2C_H_


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"

//I2C Address for the light sensor
#define I2C_ADDRESS			(0x39 << 1)

#define IIC_ERROR_STATUS 0
#define IIC_READY_STATUS 1
#define IIC_HEADER_SENT_STATUS 2
#define IIC_DATA_TRANSMISION_STATUS 3
#define IIC_DATA_SENT_STATUS 4


void I2C_init(void);
uint8_t I2C_writeData(uint8_t address, uint16_t data, uint8_t numBytes);
uint8_t I2C_writeDataArray(uint8_t address, uint8_t far* data, uint8_t numBytes);
uint8_t I2C_readDataArray(uint8_t address, uint8_t far* data, uint8_t numBytes);
uint8_t I2C_writeReadData(uint8_t address, uint8_t far* txData, uint8_t txBytes, uint8_t far* rxData, uint8_t rxBytes);
uint8_t I2C_memoryRead(uint8_t address, uint16_t memoryAddress, uint8_t addressSize, uint8_t far* data, uint8_t bytes);
uint8_t I2C_memoryWrite(uint8_t address, uint16_t memoryAddress, uint8_t addressSize, uint8_t far* data, uint8_t bytes);

void I2C_interruptHandler(void);


#endif /* I2C_H_ */

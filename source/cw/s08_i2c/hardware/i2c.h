/*
 * i2c.h
 *
 *  Created on: Sep 28, 2019
 *      Author: danao
 */

#ifndef I2C_H_
#define I2C_H_


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"

//I2C Address for the light sensor with 
//address pin floating, grounded, and high
#define I2C_ADDRESS			(0x39 << 1)			//floating   00111001 -> 0111 0010

#define IIC_ERROR_STATUS 0
#define IIC_READY_STATUS 1
#define IIC_HEADER_SENT_STATUS 2
#define IIC_DATA_TRANSMISION_STATUS 3
#define IIC_DATA_SENT_STATUS 4
#define IIC_DATA_RESTART_STATUS	5

extern unsigned char I2C_TX_DATA[16];			/* IIC Buffer */
extern unsigned char I2C_RX_DATA[16];			/* IIC Buffer */

void I2C_init(void);
uint8_t I2C_writeData(uint8_t address, uint16_t data, uint8_t numBytes);
uint8_t I2C_writeDataArray(uint8_t address, uint8_t far* data, uint8_t numBytes);
uint8_t I2C_readDataArray(uint8_t address, uint8_t far* data, uint8_t numBytes);
uint8_t I2C_writeReadData(uint8_t address, uint8_t far* txData, uint8_t txBytes, uint8_t far* rxData, uint8_t rxBytes);
uint8_t I2C_memoryRead(uint8_t address, uint16_t memoryAddress, uint8_t addressSize, uint8_t far* data, uint8_t bytes);
uint8_t I2C_memoryWrite(uint8_t address, uint16_t memoryAddress, uint8_t addressSize, uint8_t far* data, uint8_t bytes);

void I2C_interruptHandler(void);


#endif /* I2C_H_ */

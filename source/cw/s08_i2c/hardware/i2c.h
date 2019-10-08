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
//#define I2C_ADDRESS			0x29			//grounded
//#define I2C_ADDRESS			0x49			//vdd

//#define I2C_ADDRESS			0x49
//#define I2C_ADDRESS			0xE1		//1110 0001, shifted 1100 0010


#define IIC_ERROR_STATUS 0
#define IIC_READY_STATUS 1
#define IIC_HEADER_SENT_STATUS 2
#define IIC_DATA_TRANSMISION_STATUS 3
#define IIC_DATA_SENT_STATUS 4

extern unsigned char I2C_TX_DATA[16];			/* IIC Buffer */
extern unsigned char I2C_RX_DATA[16];			/* IIC Buffer */


void I2C_init(void);

void I2C_writeData(uint8_t address, uint8_t numBytes);

void I2C_interruptHandler(void);


#endif /* I2C_H_ */

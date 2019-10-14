/*
 * i2c.c
 *
 *  Created on: Sep 28, 2019
 *      Author: danao
 *  
 *  I2C Controller file for use on PA2 and PA3.  Configures
 *  the peripheral for master mode, interrupt driven.  The 
 *  approach and ISR generally follow along with the peripheral
 *  guide.  The light sensor breakout board from Adafruit is used
 *  to test the i2c.  The 7bit address is 0x39
 *  
 */


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"
#include "i2c.h"
#include "main.h"

unsigned char I2C_STEP = IIC_READY_STATUS;
unsigned char I2C_DATA_DIRECTION = 0;		//0 = read, 1 = write
unsigned char I2C_RX_LENGTH = 1;
unsigned char I2C_RX_COUNTER = 0;

unsigned char I2C_TX_LENGTH = 1;
unsigned char I2C_TX_COUNTER = 0;

unsigned char I2C_TX_DATA[16] = {0x00};
unsigned char I2C_RX_DATA[16] = {0x00};

unsigned char I2C_NO_STOP_FLAG = 0x00;
unsigned char I2C_RESTART_FLAG = 0x00;

///////////////////////////////////////////
//Configure I2C on PA2 (SDA) and PA3 (SCL).
void I2C_init(void)
{
	uint8_t dummy = 0x00;
	
	IICC1_IICEN = 1;		//i2c enable
	
	IICA = I2C_ADDRESS;		//i2c slave address, not needed
	
	//IICF - set the baud rate
	//From Table 12-4, use mult = 0x2 and ICR = 0x00
	//for 100khz
	IICF_MULT1 = 1;
	IICF_MULT0 = 0;
	
	IICF_ICR5 = 0;
	IICF_ICR4 = 0;
	IICF_ICR3 = 0;
	IICF_ICR2 = 0;
	IICF_ICR1 = 0;
	IICF_ICR0 = 0;
	
	I2C_STEP = IIC_READY_STATUS;

	IICC1_IICIE = 1;		//enable interrupts	
}


///////////////////////////////////////////////////////
//Write data to I2C address.  numBytes is the size of the
//data, either 1 or 2 bytes. Sends the MSB first
//Returns the status of the I2C.  
uint8_t I2C_writeData(uint8_t address, uint16_t data, uint8_t numBytes)
{
	uint8_t dummy = 0x00;
	
	if ((numBytes > 2) || (numBytes == 0))
		return IIC_ERROR_STATUS;

	if (numBytes == 1)
		I2C_TX_DATA[0] = data & 0xFF;
	else if (numBytes == 2)
	{
		I2C_TX_DATA[0] = (data >> 8) & 0xFF;
		I2C_TX_DATA[1] = data & 0xFF;		
	}
	
	I2C_TX_LENGTH = numBytes;
	I2C_TX_COUNTER = 0x00;
	I2C_STEP = IIC_HEADER_SENT_STATUS;
	I2C_DATA_DIRECTION = 1;

	address &= 0xFE;		//clear bit 0 for write

	IICC_IICEN = 0;
	IICC_IICEN = 1;
	
	dummy = IICS;		//clear any interrupt	
	IICS_IICIF = 1;
	
	IICC_MST = 0;		//slave
	IICS_SRW = 0;
	IICC_TX = 1;		//transmitter
	IICC_MST = 1;		//generate start condition

	//wait a bit
	for (dummy = 0 ; dummy < 5 ; dummy++);
	
	//send the first byte, starting the interrupt sequence
	IICD = address;
	
	//wait until it returns either an error or a ready state
	while (I2C_STEP > IIC_READY_STATUS){};
	
	return I2C_STEP;			
}

//////////////////////////////////////////////////
//Send numBytes over the i2c.  Contents of data array
//are copied into I2C_TX_DATA and sent over i2c.
//The approach follows along with the peripheral guide
//
uint8_t I2C_writeDataArray(uint8_t address, uint8_t far* data, uint8_t numBytes)
{	
	uint8_t dummy = 0x00;
	
	//copy numBytes into I2C_TX_DATA
	for (dummy = 0 ; dummy < numBytes ; dummy++)
		I2C_TX_DATA[dummy] = data[dummy];
	
	I2C_TX_LENGTH = numBytes;
	I2C_TX_COUNTER = 0x00;
	I2C_STEP = IIC_HEADER_SENT_STATUS;
	I2C_DATA_DIRECTION = 1;
	
	address &= 0xFE;		//clear bit 0 for write
	
	IICC_IICEN = 0;
	IICC_IICEN = 1;
	
	dummy = IICS;		//clear any interrupt	
	IICS_IICIF = 1;
	
	IICC_MST = 0;		//slave
	IICS_SRW = 0;
	IICC_TX = 1;		//transmitter
	IICC_MST = 1;		//generate start condition
	
	//wait a bit
	for (dummy = 0 ; dummy < 5 ; dummy++);

	//start the transmission and the interrupt sequence.
	IICD = address;

	//wait until it returns either an error or a ready state
	while (I2C_STEP > IIC_READY_STATUS){};
	
	return I2C_STEP;		
}


/////////////////////////////////////////////////////
//Read numBytes into data array from address.  
//Uses I2C_RX_DATA to read the data, then copies
//it when returning if the status is ready
uint8_t I2C_readDataArray(uint8_t address, uint8_t far* data, uint8_t numBytes)
{
	uint8_t temp;
	
	I2C_RX_LENGTH = numBytes;	
	I2C_RX_COUNTER = 0;
	I2C_STEP = IIC_HEADER_SENT_STATUS;
	I2C_DATA_DIRECTION = 0;				//receiver
	
	//set the address with bit 0 high
	address &= 0xFE;
	address |= 0x01;		//set as read
	
	temp = IICS;			//clear pending interrupts
	IICS_IICIF = 1;
	
	IICC1_TXAK = 0x00;		//ack signal is sent after receiving one data byte
	IICC_TX = 1;			//transmitter to send address but with add bit 0 high
	
	//generate the start or a restart
	if (I2C_RESTART_FLAG == 1)
	{
		IICC_RSTA = 1;
		I2C_RESTART_FLAG = 0;
	}
	else
	{
		IICC_MST = 1;			//generate start condition		
	}
	
	//wait a bit
	for (temp = 0 ; temp < 5 ; temp++);
	
	//send the address
	IICD = address;
	
	//wait until it returns either an error or a ready state
	while (I2C_STEP > IIC_READY_STATUS){};

	//copy the data if it's valid and no errors
	if (I2C_STEP == IIC_READY_STATUS)
	{
		//copy the data from the rx buffer into data
		for (temp = 0 ; temp < numBytes ; temp++)
			data[temp] = I2C_RX_DATA[temp];
	}
	
	return I2C_STEP;
}





//////////////////////////////////////////////////////
//I2C_writeReadData - 
//write txBytes of txData to address, generate 
//a restart condition, then read rxBytes into rxData
//return the status.  Two flags are used to indicate
//no stop condition after completing the write, and 
//generate a restart prior to reading.
//
uint8_t I2C_writeReadData(uint8_t address, uint8_t far* txData, uint8_t txBytes, uint8_t far* rxData, uint8_t rxBytes)
{
	uint8_t status = 0x00;					//status of the i2c transfer

	//no stop condition generated after completing write cycle
	I2C_NO_STOP_FLAG = 1;

	//write step - waits until all tx bytes are complete
	status = I2C_writeDataArray(address, txData, txBytes);
	
	//clear the no stop flag
	I2C_NO_STOP_FLAG = 0;
	
	//generate a restart in the read segment
	I2C_RESTART_FLAG = 1;

	//read step - waits until all rx bytes are complete
	status = I2C_readDataArray(address, rxData, rxBytes);

	//clear the flags
	I2C_RESTART_FLAG = 0;
	I2C_NO_STOP_FLAG = 0;
	
	return status;
}


///////////////////////////////////////////////////////////
//I2C_memoryRead
//Reads data from I2C at a memory address. 
//Sends addressSize bytes as a write to address, generates
//a restart condition, then reads rxBytes into rxData array
//Memory address is sent to the I2C MSB first.  Memory
//address size can be either 1 or 2 bytes.  Returns the
//status of the I2C
uint8_t I2C_memoryRead(uint8_t address, uint16_t memoryAddress, uint8_t addressSize, uint8_t far* data, uint8_t bytes)
{
	uint8_t status = 0x00;					//status of the i2c transfer

	//no stop condition generated after completing write cycle
	I2C_NO_STOP_FLAG = 1;

	//write step - waits until all tx bytes are complete
	status = I2C_writeData(address, memoryAddress, addressSize);

	if (status == IIC_ERROR_STATUS)
		return status;
	
	//clear the no stop flag
	I2C_NO_STOP_FLAG = 0;
	
	//generate a restart in the read segment
	I2C_RESTART_FLAG = 1;

	//read step - waits until all rx bytes are complete
	status = I2C_readDataArray(address, data, bytes);

	//clear the flags
	I2C_RESTART_FLAG = 0;
	I2C_NO_STOP_FLAG = 0;
	
	return status;	
}


/////////////////////////////////////////////////////////
//I2C_memoryWrite
//Write data to a memory address over I2C bus. 
//Sends addressSize bytes as a write to address, followed
//by a write data of length bytes.  Memory address is 
//sent to the I2C MSB first.  Memory address size can be
//either 1 or 2 bytes.  Returns the status of the I2C
//Similar to a write array, but copy the memory address
//bytes into the array. 
//
uint8_t I2C_memoryWrite(uint8_t address, uint16_t memoryAddress, uint8_t addressSize, uint8_t far* data, uint8_t bytes)
{
	uint8_t dummy = 0x00;
	uint8_t count = 0x00;
	
	//copy the memoryAddress bytes into I2C_TX_DATA
	if (addressSize == 1)
	{
		I2C_TX_DATA[count++] = (memoryAddress & 0xFF);
	}
	else if (addressSize == 2)
	{
		I2C_TX_DATA[count++] = (memoryAddress >> 8) & 0xFF;
		I2C_TX_DATA[count++] = (memoryAddress & 0xFF);
	}
	
	//copy the data bytes into I2C_TX_DATA
	for (dummy = 0 ; dummy < bytes ; dummy++)
		I2C_TX_DATA[count++] = data[dummy];
	
	I2C_TX_LENGTH = (bytes + addressSize);
	I2C_TX_COUNTER = 0x00;
	I2C_STEP = IIC_HEADER_SENT_STATUS;
	I2C_DATA_DIRECTION = 1;
	
	address &= 0xFE;		//clear bit 0 for write
	
	IICC_IICEN = 0;
	IICC_IICEN = 1;
	
	dummy = IICS;		//clear any interrupt	
	IICS_IICIF = 1;
	
	IICC_MST = 0;		//slave
	IICS_SRW = 0;
	IICC_TX = 1;		//transmitter
	IICC_MST = 1;		//generate start condition
	
	//wait a bit
	for (dummy = 0 ; dummy < 5 ; dummy++);
	
	IICD = address;		//send the first byte - address

	//wait until it returns either an error or a ready state
	//Should add a counter here, a timeout, etc.
	while (I2C_STEP > IIC_READY_STATUS){};
	
	return I2C_STEP;
}





///////////////////////////////////////////////
//I2C Interrupt Service Routine
//Master mode transmitter and receiver
//The ISR generally follows the peripheral guide.
//
void I2C_interruptHandler(void)
{
	unsigned char temp;

	temp = IICS;						//clear the interrupt flag
	IICS_IICIF = 1;
	
	//Interrupt Source - Arbitration Lost	 
	if(IICS_ARBL==1)
	{
		IICS_ARBL= 1;					//clear the flag
		IICC_MST = 0;					//generate the stop condition
		I2C_STEP = IIC_ERROR_STATUS;	//update the status
		return;
	}

	if(IICC_MST==1)		
	{
		//if not ack and device is transmitter, return error
		//previously, it was both rx and tx, which is not right
		//for some i2c devices
		if((IICS_RXAK==1) && (IICC1_TX == 1))
		{
			IICC_MST = 0;					//generate the stop condition
			I2C_STEP = IIC_ERROR_STATUS;	//update the status
			return;
		}

		/////////////////////////////////////////////////////
		//I2C Status - Header Sent
		//address byte sent - update the direction and status
		if(I2C_STEP == IIC_HEADER_SENT_STATUS)
		{
			IICC_TX = I2C_DATA_DIRECTION;				//set the direction
			I2C_STEP = IIC_DATA_TRANSMISION_STATUS; 	//update the status
		}

		//////////////////////////////////////////////////////
		//I2C Status - Transmission Status - Data is being sent
		//Master is a transmitter or a receiver
		//
		if(I2C_STEP == IIC_DATA_TRANSMISION_STATUS)
		{
			///////////////////////////////////////
			//Transmitter
			if(IICC_TX==1)
			{
				IICD = I2C_TX_DATA[I2C_TX_COUNTER];		//send the data
				I2C_TX_COUNTER++;						//increment the counter

				//Last Byte?  The next interrupt will be generated
				//when last byte is complete with a status sent
				//which will then generate the stop condition... etc
				if(I2C_TX_LENGTH <= I2C_TX_COUNTER)
				{
					I2C_STEP=IIC_DATA_SENT_STATUS;
				}
				
				return;
			}

			///////////////////////////////////////////////////
			//Receiver - master reads data from the slave.
			//The ack bit is pulled low by the master receiver
			//for all bytes except the last one.
			else
			{
				//read only 1 byte
				if (I2C_RX_LENGTH == 1)
				{
					IICC_TXAK = 1;						//master drives ack bit high
					I2C_RX_DATA[I2C_RX_COUNTER] = IICD;	//read the data into rx data				
					I2C_RX_COUNTER++;					//increment the counter					
					I2C_STEP=IIC_DATA_SENT_STATUS;		//update the status- - all data sent
				}
				
				//read 2 or more bytes
				else
				{
					//Uncomment this to set all receive bytes read by 
					//master to set the ack bit high.  Otherwise, only
					//the last byte received will have the ack bit set
					//high by the master
					
					//IICC_TXAK = 1;							//master drives ack bit high
					I2C_STEP = IIC_DATA_TRANSMISION_STATUS; 	//update the status
					
					//the next byte is the last one, master receiver
					//sets the ack bit high
					if((I2C_RX_COUNTER+1) == I2C_RX_LENGTH)
						IICC_TXAK = 1;
					
					I2C_RX_DATA[I2C_RX_COUNTER] = IICD;		//read the data       	         	 
					I2C_RX_COUNTER++;						//increment the counter

					//update the status to complete
					if(I2C_RX_LENGTH <= I2C_RX_COUNTER)
						I2C_STEP=IIC_DATA_SENT_STATUS;
				}
				
				return;
			}
		}
	
		///////////////////////////////////////////////////
		//I2C Status - Tx / Rx complete, return to ready state
		//Generate the stop condition
		if(I2C_STEP==IIC_DATA_SENT_STATUS)
		{
			I2C_STEP=IIC_READY_STATUS;		//I2C Ready state
			temp = IICS;					//Clear the interrupt
			IICS_IICIF=1;
	
			IICC_TX=0;
			IICS_SRW=0;
			
			//generate the stop condition if the flag is not set
			if (I2C_NO_STOP_FLAG == 0)		
				IICC_MST=0;
			
			return;
		}		
	}	
}


////////////////////////////////////////////////
//I2C Interrupt Service Routine.  
//
void interrupt VectorNumber_Viic iic_isr(void)
{
	//clear the interrupt flag by writing a 1
	IICS_IICIF = 1;
	
	I2C_interruptHandler();
}





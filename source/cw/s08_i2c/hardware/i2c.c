/*
 * i2c.c
 *
 *  Created on: Sep 28, 2019
 *      Author: danao
 *  
 *  I2C Controller file for use on PA2 and PA3.
 *  Configures the peripheral for master mode.
 *  
 *  light sensor i2c 7bit - 0x39
 *  
 */


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"
#include "i2c.h"
#include "main.h"

unsigned char I2C_STEP = IIC_READY_STATUS;
unsigned char I2C_DATA_DIRECTION = 0;      /* 1 Transmit, 0 Read */
unsigned char I2C_RX_LENGTH = 1;
unsigned char I2C_RX_COUNTER = 0;

unsigned char I2C_TX_LENGTH = 1;
unsigned char I2C_TX_COUNTER = 0;

unsigned char I2C_TX_DATA[16] = {0x00};
unsigned char I2C_RX_DATA[16] = {0x00};


///////////////////////////////////////
//Configure I2C on PA2 (SDA) and PA3 (SCL).
void I2C_init(void)
{
	uint8_t dummy = 0x00;
	
	IICC1_IICEN = 1;		//i2c enable
	
	//IICA - set the slave address - assume 0x50
	IICA = I2C_ADDRESS;
	
	//IICF - set the baud rate
	//From Table 12-4, use mult = 0x2 and ICR = 0x00
	//
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




//////////////////////////////////////////////////
//Send num bytes over the i2c.  byte data is
//assumed to be in I2C_DATA
//This follows along with the example i2c in the
//peripheral guide
uint8_t I2C_writeData(uint8_t address, uint8_t far* data, uint8_t numBytes)
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
	
	IICD = address;		//send the first byte - address
	
	//wait until it returns either an error or a ready state
	//Should add a counter here, a timeout, etc.
	while (I2C_STEP > IIC_READY_STATUS){};
	
	return I2C_STEP;
		
}


/////////////////////////////////////////////////////
//Read numBytes into data array from address.  
//Uses I2C_RX_DATA to read the data, then copies
//it when returning if the status is ready
uint8_t I2C_readData(uint8_t address, uint8_t far* data, uint8_t numBytes)
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
	IICC_MST = 1;			//generate start condition
	
	//wait a bit
	for (temp = 0 ; temp < 5 ; temp++);
	
	//send the address
	IICD = address;
	
	//wait until it returns either an error or a ready state
	//Should add a counter here, a timeout, etc.
	while (I2C_STEP > IIC_READY_STATUS){};
	
	if (I2C_STEP == IIC_READY_STATUS)
	{
		//copy the data from the rx buffer into data
		for (temp = 0 ; temp < numBytes ; temp++)
			data[temp] = I2C_RX_DATA[temp];
	}
	
	return I2C_STEP;
}


void I2C_interruptHandler(void)
{
	unsigned char Temp;

	Temp = IICS;              /* ACK the interrupt */
	IICS_IICIF = 1;
	 
	if(IICS_ARBL==1)
	{         /* Verify the Arbitration lost status */	     
		IICS_ARBL= 1;
		IICC_MST = 0;	
		I2C_STEP = IIC_ERROR_STATUS;
		return;     
	}										       /* If Arbitration is OK continue */  

	if(IICC_MST==1)		
	{
		//if not ack and device is transmitter, return error
		//previously, it was both rx and tx, which is not right
		//for some i2c devices
		if((IICS_RXAK==1) && (IICC1_TX == 1))
		{
			IICC_MST = 0;					//generate the stop condition
			I2C_STEP = IIC_ERROR_STATUS;
			return;
		}
	
		//the address byte is sent, set the direction and
		//update the status
		if(I2C_STEP == IIC_HEADER_SENT_STATUS)
		{
			IICC_TX = I2C_DATA_DIRECTION;				//set the direction
			I2C_STEP = IIC_DATA_TRANSMISION_STATUS; 	//update the status

			//if rx, read the first byte, set the ack or nack
			//depending on if more bytes to be read
			if(IICC_TX==0)
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
					
					//IICC_TXAK = 1;						//master drives ack bit high
					I2C_STEP = IIC_DATA_TRANSMISION_STATUS; 	//update the status
					
					//the next byte is the last one, master reciever
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
	
		if(I2C_STEP == IIC_DATA_TRANSMISION_STATUS)
		{	 /* If byte transmision is in progress.*/
			if(IICC_TX==1)
			{				               /* If Master is sending data to slave */
				IICD = I2C_TX_DATA[I2C_TX_COUNTER];	     /* Send the next byte */	
				I2C_TX_COUNTER++;
				
				if(I2C_TX_LENGTH <= I2C_TX_COUNTER)
				{
					I2C_STEP=IIC_DATA_SENT_STATUS;     /* Mark we are done sending Bytes */   	  
				}
				
				return;        	 								     /* wait until last byte sent      */
			}
			
			else
			{										               /* If master is reading data from slave */	
				if((I2C_RX_COUNTER+1) == I2C_RX_LENGTH)  /* Master should not ACK the last byte */
					IICC_TXAK = 1;							   /* to indicate end of transfer         */
	
				I2C_RX_DATA[I2C_RX_COUNTER] = IICD;	     /* Read the next byte */       	         	 
				I2C_RX_COUNTER++;
				 
				if(I2C_RX_LENGTH <= I2C_RX_COUNTER)
				{
					I2C_STEP=IIC_DATA_SENT_STATUS;   /* Mark we are done sending Bytes */   	  
				}
				
				return;           	    					 /* Return until next byte is read */
			}
		}
	
		//data transmit / receive complete, return to ready
		//status, generate the stop condition
		if(I2C_STEP==IIC_DATA_SENT_STATUS)
		{	       /* We are done with the transmition.*/ 	
			I2C_STEP=IIC_READY_STATUS;	             /* Reset our status flag            */
			Temp = IICS;                            /* ACK the interrupt                */
			IICS_IICIF=1;
	
			IICC_TX=0;
			IICS_SRW=0;
			IICC_MST=0;
			   /* Generate a stop condition        */        	  
			return;
		}
	}
	

	//slave state
	else
	{  
		//first byte?
		if(I2C_STEP <= IIC_READY_STATUS)
		{
			I2C_STEP = IIC_DATA_TRANSMISION_STATUS;
			//transmit reception status
			IICC_TX = IICS_SRW;
			I2C_TX_COUNTER = 1;

			//if rx, read the data register
			if(IICC_TX==0)
			{
				Temp = IICD;
				return;
			}
		}
		
		if(IICS_TCF==1)
		{
			if(IICC_TX == 0)
			{
				//store read data into rx buffer              
				I2C_RX_DATA[I2C_RX_COUNTER]=IICD;
				I2C_RX_COUNTER++;
				return;          
			}
			
			//data sent by slave to master
			else
			{
				if(IICS_RXAK==1)					
				{
					//end transmission for nACK
					IICC_TX = 0;
					Temp = IICD;
					I2C_STEP = IIC_READY_STATUS;
					return;
				}
		
				IICD = I2C_TX_DATA[I2C_TX_COUNTER];
				I2C_TX_COUNTER++;
				return;          
			}
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





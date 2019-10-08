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
unsigned char I2C_LENGTH = 1;
unsigned char I2C_COUNTER = 0;

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
void I2C_writeData(uint8_t address, uint8_t numBytes)
{	
	uint8_t dummy = 0x00;
	
	I2C_LENGTH = numBytes;
	I2C_COUNTER = 0x00;
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
	
	return;
	
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
	{          /* If we are the IIC Master */	
		if(IICS_RXAK==1)
		{      /* Verify if byte sent was ACK */
			IICC_MST = 0;
			I2C_STEP = IIC_ERROR_STATUS;	
			return;
		}
	
		if(I2C_STEP == IIC_HEADER_SENT_STATUS)
		{ /* Header Sent */
			IICC_TX = I2C_DATA_DIRECTION;
			I2C_STEP = IIC_DATA_TRANSMISION_STATUS; 
	
			if(IICC_TX==0)
			{  /* If we are reading data clock in first slave byte */
				Temp = IICD;
				return;
			}
		}
	
		if(I2C_STEP == IIC_DATA_TRANSMISION_STATUS)
		{	 /* If byte transmision is in progress.*/
			if(IICC_TX==1)
			{				               /* If Master is sending data to slave */
				IICD = I2C_DATA[I2C_COUNTER];	     /* Send the next byte */	
				I2C_COUNTER++;
				
				if(I2C_LENGTH <= I2C_COUNTER)
				{
					I2C_STEP=IIC_DATA_SENT_STATUS;     /* Mark we are done sending Bytes */   	  
				}
				
				return;        	 								     /* wait until last byte sent      */
			}
			
			else
			{										               /* If master is reading data from slave */	
				if((I2C_COUNTER+1) == I2C_LENGTH)  /* Master should not ACK the last byte */
					IICC_TXAK = 1;							   /* to indicate end of transfer         */
	
				I2C_DATA[I2C_COUNTER] = IICD;	     /* Read the next byte */       	         	 
				I2C_COUNTER++;
				 
				if(I2C_LENGTH <= I2C_COUNTER)
				{
					I2C_STEP=IIC_DATA_SENT_STATUS;   /* Mark we are done sending Bytes */   	  
				}
				
				return;           	    					 /* Return until next byte is read */
			}
		}
	
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
	
	
	else
	{			 /*  SLAVE OPERATION  */  
		if(I2C_STEP <= IIC_READY_STATUS)
		{	 				 /* If it is the first byte tranmited */
			I2C_STEP = IIC_DATA_TRANSMISION_STATUS;
			IICC_TX = IICS_SRW;				           /* Set the transmision reception status */
			I2C_COUNTER = 1;
		
			/* If we are receiving data read IIC1D to get free bus and get the next byte */
			if(IICC_TX==0)
			{
				Temp = IICD;
				return;
			}
		}
		
		if(IICS_TCF==1)
		{
			if(IICC_TX == 0)
			{	        /* If data is received store it on the buffer */              
				I2C_DATA[I2C_COUNTER]=IICD;
				I2C_COUNTER++;
				return;          
			}
			
			else
			{	                      /* Data sent by the slave */              		
				if(IICS_RXAK==1)
				{      /* If byte is not ACK end transmision. */
					IICC_TX = 0;
					Temp = IICD;
					I2C_STEP = IIC_READY_STATUS;
					return;
				}
		
				IICD = I2C_DATA[I2C_COUNTER];
				I2C_COUNTER++;
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





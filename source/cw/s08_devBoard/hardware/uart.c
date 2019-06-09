/*
 * uart.c
 *
 *  Created on: May 11, 2019
 *      Author: danao
 * 
 *  Header file for controlling the UART peripheral.
 *  Configures the UART at variable baud rate, 8 bits, no
 *  parity, 1 stop bit.  Rx interrupts are enabled
 *  
 *  Pinout: 
 *  PB1 - TX - Pin 19
 *  PB0 - RX - Pin 20
 *  
 *  
 */



#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include <string.h>

#include "config.h"
#include "uart.h"


///////////////////////////////////////////////
//UART Variables
volatile uint8_t rxFlag = 0x00;				//set if complete instruction is ready
volatile uint8_t rxIndex = 0x00;			//index in the buffer
volatile uint8_t rxBuffer[UART_BUFFER_SIZE];//data buffer


/////////////////////////////////////////////////////
//Configure the uart peripheral for 9600 baud, tx/rx,
//rx interrupt enabled, 8bit, no parity, 1 stop bit
//See Section 14.2.1
//Baud rate = bus speed / divider.
//No idea what the bus speed is. - 264khz?
//through trial and error, rates of 115200 and 
//57600 don't work when using internal clock
//
//When using the default bus speed, available
//rates 9600, 19200, and 38400
void UART_init(BaudRate_t rate)
{
	char* result = memset(rxBuffer, 0x00, UART_BUFFER_SIZE);	//clear the buffer
	rxIndex = 0x00;								//reset the index
	rxFlag = 0x00;								//reset the flag
	
	//default rate = 9600
	SCIBDH = 0x00;
	SCIBDL = 0x1C;		//divider 28
	
	switch(rate)
	{
		case BAUD_RATE_9600:	SCIBDL = 0x1C;	break;
		case BAUD_RATE_19200:	SCIBDL = 0x0E;	break;
		case BAUD_RATE_38400:	SCIBDL = 0x07;	break;
		default: 				SCIBDL = 0x1C;	break;
	}
	
	//SCIC1 - Control register 1
	SCIC1_LOOPS = 0x00;		//rx and tx use separate pins
	SCIC1_SCISWAI = 0x00;	//clocks continue to run in wait mode
	SCIC1_RSRC = 0x00;		//no meaning
	SCIC1_M = 0x00;			//8 bit mode
	SCIC1_WAKE = 0x00;		//idle line wakeup
	SCIC1_ILT = 0x00;		//idle starts after start bit
	SCIC1_PE = 0x00;		//no parity
	SCIC1_PT = 0x00;		//even parity - NA
	
	//SCIC2 - Control Register 2
	SCIC2_TIE = 0x00;		//transmitter interrupt disable
	SCIC2_TCIE = 0x00;		//transmit complete interrupt disable
	SCIC2_RIE = 0x01;		//receiver interrupt enable
	SCIC2_ILIE = 0x00;		//idle line interrupt disable
	SCIC2_TE = 0x01;		//enable transmitter
	SCIC2_RE = 0x01;		//enable receiver
	SCIC2_RWU = 0x00;		//normal control
	SCIC2_SBK = 0x00;		//dont send a break - normal
	
	//SCIS1 - status register - read only flags	
	//SCIS2 - status register - read and write

	//That's all we need so far	
	
}


///////////////////////////////////////////////////
//Transmit 1 byte over the uart peripheral
//put data into the SCID register and poll
//the transmit complete flag
void UART_tx(uint8_t data)
{
	SCID = data;			//put data into register
	while (!SCIS1_TC){};	//poll transmit complete flag
}


void UART_sendString(char* msg)
{
	uint8_t index = 0x00;	
	while (msg[index])
	{
		UART_tx(msg[index]);
		index++;
	}
}


void UART_sendStringLength(uint8_t* buffer, int len)
{
	uint8_t i = 0x00;
	
	for (i = 0 ; i < len ; i++)
		UART_tx(buffer[i]);
}



////////////////////////////////////////////
//Function called from polling loop to process
//the incoming data.  
void UART_processCommand(void)
{
	char* result = 0x00;
	
	//do something with the buffer
	UART_sendString("RX MSG: ");
	UART_sendStringLength((uint8_t*)rxBuffer, rxIndex);
	
	//reset the buffer
	result = memset(rxBuffer, 0x00, UART_BUFFER_SIZE);
	rxIndex = 0x00;
}



///////////////////////////////////////////////
//Interrupt Service Routine for RX
//Interrupt is generated when RDRF flag in the 
//SCIS1 register is set.  Clear the flag by reading
//the SCIS1 register, then read the data register SCID
void interrupt VectorNumber_Vscirx uart_rx_isr(void)
{
	uint8_t dummy = 0x00;
	uint8_t data = 0x00;
	
	dummy = SCIS1;		//read the status reg to clear the flag
	data = SCID;		//read the data register

	if (!rxFlag)
		UART_InterruptHandler(data);	
}


///////////////////////////////////////////////////////
//UART_InterruptHandler
//Called when the UART receives a character.  
void UART_InterruptHandler(uint8_t data)
{		
	if (rxIndex < (UART_BUFFER_SIZE - 1))
	{
		rxBuffer[rxIndex] = data;		//put data into buffer
		rxIndex++;						//increment the index
		
		if (data == '\n')				//end of line??
		{
			rxBuffer[rxIndex] = 0x00;		//end the line
			rxFlag = 1;						//set the flag
		}
	}
	
	else
	{
		//overrun
		char* result = memset(rxBuffer, 0x00, UART_BUFFER_SIZE);
		rxIndex = 0x00;
		rxFlag = 0x00;
	}
	
}





/*
 * uart.h
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

#ifndef UART_H_
#define UART_H_

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"

#define UART_BUFFER_SIZE		32


typedef enum
{
	BAUD_RATE_9600,
	BAUD_RATE_19200,
	BAUD_RATE_38400
}BaudRate_t;


//rxFlag - set from the interrupt handler when receives \n
extern volatile uint8_t rxFlag;

void UART_init(BaudRate_t rate);
void UART_tx(uint8_t data);
void UART_sendString(char* msg);
void UART_sendStringLength(uint8_t* buffer, int len);
void UART_processCommand(void);
void UART_InterruptHandler(uint8_t data);



#endif /* UART_H_ */




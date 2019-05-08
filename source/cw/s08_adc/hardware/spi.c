/*
 * spi.c
 *
 *  Created on: Apr 12, 2019
 *      Author: danao
 *  
 *  Configure the SPI interface on PB2 - PB5
 *  PB2 - SCK		Pin 18
 *  PB3 - MOSI		Pin 17
 *  PB4 - MISO		Pin 12
 *  PB5 - SS - 		Pin 11 - Configure as normal IO
 *  
 */

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"
#include "spi.h"


///////////////////////////////////////////
//SPI initialization
//Configure as master, MSB first, idle clock
//low, data on a leading edge.  SS pin configured
//as normal IO.  SPI clock configured to 1mhz
//NOTE: There are unusually long delays between 
//bytes when sending an array.  ie, doubling the clock
//speed does not double the rate.
//
void SPI_init(void)
{	
	//Configure the IO for SPI	
	PTBDD |= BIT5;			//SS pin as output
	PTBD |= BIT5;			//SS initially high
	
	//SPIC1 Register - See Table 15-1
	SPIC1_SPIE = 0x00;		//disable interrupt
	SPIC1_SPE = 0x00;		//disable SPI - enable later
	SPIC1_SPTIE = 0x00;		//disable tx interrupts
	SPIC1_MSTR = 1;			//set as master mode
	SPIC1_CPOL = 0;			//idle clock low
	SPIC1_CPHA = 0;			//data on leading edge
	SPIC1_SSOE = 0x00;		//SS normal IO
	SPIC1_LSBFE = 0x00;		//MSB first
	
	//SPIC2 Register - See Table 15-3
	SPIC2_MODFEN = 0x00;	//SS as normal IO
	SPIC2_BIDIROE = 0x00;	//NA for 4 wire configuration
	SPIC2_SPISWAI = 0x00;	//SPI continues to opperate in stop mode
	SPIC2_SPC0 = 0x00;		//separate pins for input and output
	
	//SPIBR - Configure the baud rate - Tables 15-4 and 15-5
	//For now, assume prescale = 2, rate divider = 2
	//Top bits = 0001 (Note: Bit 7 not used, set as 0)
	SPIBR_SPPR2 = 0x00;
	SPIBR_SPPR1 = 0x00;
	SPIBR_SPPR0 = 0x01;
		
	//Lower bits = 0000 - missing bit 3 from header
	SPIBR_SPR2 = 0x00;
	SPIBR_SPR1 = 0x00;
	SPIBR_SPR0 = 0x00;

	//enable the SPI
	SPIC1_SPE = 0x01;		//enable the SPI		
}


void SPI_select(void)
{
	PTBD &=~ BIT5;
}

void SPI_deselect(void)
{
	PTBD |= BIT5;	
}

/////////////////////////////////////////////
//SPI send one byte
//
void SPI_tx(uint8_t data)
{
	uint8_t temp = 0x00;
	while(!SPIS_SPTEF){};	//wait while tx buffer is not empty
	SPID = data;			//write data
	while(!SPIS_SPTEF){};	//wait while tx buffer is not empty	
	while(!SPIS_SPRF){};	//wait while the rx buffer is not full;	
	temp = SPID;			//read the data register to clear the tx flag
}


///////////////////////////////////
//SPI receive one byte
//poll the status bit - SPRF - SPI read buffer full = 1
uint8_t SPI_rx(void)
{
	uint8_t result = 0x00;	
	while(!SPIS_SPTEF){};	//wait while tx buffer is not empty
	SPID = 0xFF;			//send dummy byte	
	while(!SPIS_SPTEF){};	//wait while tx buffer is not empty	
	while(!SPIS_SPRF){};	//wait while the rx buffer is not full;
	result = SPID;			//read the result
	return result;	
}

/////////////////////////////////////////////
//SPI send one byte - put into buffer
//and wait until tx buffer is empty
void SPI_write(uint8_t data)
{
	SPI_select();
	SPI_tx(data);
	SPI_deselect();
}

uint8_t SPI_read(void)
{
	uint8_t result = 0x00;
	SPI_select();
	result = SPI_rx();
	SPI_deselect();
	return result;
}

void SPI_writeArray(uint8_t* data, uint16_t length)
{
	uint16_t i = 0x00;
	SPI_select();
	for (i = 0 ; i < length ; i++)
		SPI_tx(data[i]);

	SPI_deselect();
}

void SPI_readArray(uint8_t* data, uint16_t length)
{
	uint16_t i = 0x00;
	SPI_select();
	for (i = 0 ; i < length ; i++)
		data[i] = SPI_rx();
	
	SPI_deselect();	
}



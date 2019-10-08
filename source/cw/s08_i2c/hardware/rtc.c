/*
 * rtc.c
 *
 *  Created on: Apr 11, 2019
 *      Author: danao
 *      
 *  Real Time Counter Control File.
 *  The purpose of this file is to initialise
 *  the real time counter module to trigger an 
 *  interrupt at a given frequency.
 *  
 *  Note:  The interrupt notation shown below 
 *  compiles and appears to function properly
 *  yet gives a syntax error.  
 */

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include <stddef.h>
#include "config.h"
#include "rtc.h"

//global time tick for delay function
volatile unsigned long gTimeTick = 0x00;

///////////////////////////////////////////
//Configure the Real Time Counter module
//This function sets up the RTC module to 
//generate an interrupt at a desired RTC_Frequency_t
//from 1hz to 1000hz.  The module is configured to 
//use the 1khz internal low power clock source.
//
void RTC_init_internal(RTC_Frequency_t freq)
{
	//RTCSC - enable interrupt, use 1khz clock
	unsigned char value = 0x10;
	
	gTimeTick = 0x00;

	
	//configure the prescale bits
	switch(freq)
	{
		case RTC_FREQ_1HZ: 		value |= 0x0F;		break;
		case RTC_FREQ_10HZ: 	value |= 0x0D;		break;
		case RTC_FREQ_100HZ: 	value |= 0x0B;		break;
		case RTC_FREQ_1000HZ: 	value |= 0x08;		break;
		default: 				value |= 0x0F;		break;		
	}

	RTCMOD = 0x00;
	RTCSC = value;
}


///////////////////////////////////////////
//RTC_init_external
//Configure the RTC peripheral to trigger on 
//the external clock source.  Input xtal rate 16mhz
//and target interrupt rate is 8khz.
//Registers:
//RTCSC - interrupt, clocks...
//RTCMOD - modulus - set interrupt freq.
//RTCCNT - current count - read only
//The following sets up an interrupt to trigger
//at a rate of 8khz.  This is based on a 16mhz xtal
//and external clock configured as shown in main.c
void RTC_init_external(void)
{
	gTimeTick = 0x00;
	
	//RTCSC
	RTCSC_RTIF = 1;		//clear any interrupts
	RTCSC_RTCLKS1 = 0;	//01 - use external clock
	RTCSC_RTCLKS0 = 1;	//01 - use external clock	
	RTCSC_RTIE = 1;		//enable real time counter interrupt
	
	//RTC Divider Bits - Using 16mhz xtal
	//See Table XX in the datasheet
	//Divide = 8 results in about 16khz interrupt
	//Divide = 9 results in about 8khz interrupt
	//Divide = 10 results in about 3.2khz
	//Divide = 11 results in about 1.6khz
	//Divide = 12 results in about 800hz
	
	RTCSC_RTCPS3 = 1;
	RTCSC_RTCPS2 = 0;
	RTCSC_RTCPS1 = 0;
	RTCSC_RTCPS0 = 1;	
}


unsigned long RTC_getTimeTick(void)
{
	return gTimeTick;
}

/////////////////////////////////////////////
//Delay in units of timebase for RTC interrupt
//ie, For RTC configured to 1khz timeout, units in ms
void RTC_delay(unsigned int delay)
{
	volatile unsigned long temp = delay + gTimeTick;
	while (temp > gTimeTick){};	
}


////////////////////////////////////////////////
//RTC Interrupt Routine
//Syntax is the following:
//void interrupt VectorNumber function(void)
//See mc9s08qe8.h for vector definitions
//RTC interrupt maps to 24, address = 0xFFCEU
//
//I have no idea what the syntax error is.  It compiles
//and runs ok.
void interrupt VectorNumber_Vrtc rtc_isr(void)
{
	RTCSC_RTIF = 1;			//clear the interrupt flag	
	gTimeTick++;			//increment the time tick	
}




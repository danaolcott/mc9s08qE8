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
volatile unsigned int gTimeTick = 0x00;

////////////////////////////////////////
//Configure the Real Time Counter module
//Configure the module to timeout and generate
//an interrupt at specified rate.  Uses the 
//1khz low power clock as the source.  See 
//Table 13-2 for prescale values.
//RTCSC = 0x1 ....
//	where .... are the prescale bits
//	1hz  = 0x0F
//	1khz = 0x08
//  100hz = 0x0B
//  10hz = 0x0D
//
//RTCMOD - 0x00

void RTC_init(RTC_Frequency_t freq)
{
	//RTCSC - enable interrupt, use 1khz clock
	unsigned char value = 0x10;
	
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




/////////////////////////////////////////////
//Delay in units of timebase for RTC interrupt
//ie, For RTC configured to 1khz timeout, units in ms
//
void RTC_delay(unsigned int delay)
{
	volatile unsigned int temp = delay + gTimeTick;
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

	//do something...
}




/*
 * rtc.h
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
 *      
 */

#ifndef RTC_H_
#define RTC_H_

//////////////////////////////////////
//Enum for setting the timeout
//frequency of the RTC real time clock
typedef enum
{
	RTC_FREQ_1HZ,
	RTC_FREQ_10HZ,
	RTC_FREQ_100HZ,
	RTC_FREQ_1000HZ,
}RTC_Frequency_t;


extern unsigned char mToggleInterval;


void RTC_init(RTC_Frequency_t freq);
void RTC_delay(unsigned int delay);


#endif /* RTC_H_ */

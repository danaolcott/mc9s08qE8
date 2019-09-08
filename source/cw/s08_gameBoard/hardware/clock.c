/*
 * clock.c
 *
 *  Created on: Aug 3, 2019
 *      Author: danao
 */

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"
#include "clock.h"

////////////////////////////////////////////////////
//Clock_init()
//Configure the clock to use the external oscillator
//as the clock source.  Note:  the default is using the
//internal source, which I think is 1mhz, and sets up the
//RTC for 1khz timeout.
//
//From the datasheet:
//ICSC1 - internal clock source control register 1
//ICSC2 - internal clock source control register 2
//ICSTRM - trim register
//
void Clock_init(void)
{
	////////////////////////////////////////////
	//ICSC1 Register
	//CLKS bits - 10 - external ref clock selected
	ICSC1_CLKS1 = 1;
	ICSC1_CLKS0 = 0;
	
	//RDIV - 100 - divider 512, high range, high gain 16mhz
	//16mhz / 512 = 31.25khz, OK
	ICSC1_RDIV2 = 1;
	ICSC1_RDIV1 = 0;
	ICSC1_RDIV0 = 0;
	
	//IREFS - internal reference select - 0 is external
	ICSC1_IREFS = 0;

	////////////////////////////////////////////////////
	//ICSC2 Register
	//Bis divider - direct impact on the speed at which
	//the device runs.  about 350khz for divider = 1 (00)
	//and about 50mhz for a divider = 8 (11)
	//Bus divider = 00 - clock divide 1
 	ICSC2_BDIV1 = 0;
	ICSC2_BDIV0 = 0;

	//set the range and gain based on speed of ext osc.
	ICSC2_RANGE = 1;		//high range
	ICSC2_HGO = 1;			//high gain
	
	ICSC2_LP = 1;			//FLL is disabled in bypass mode
	ICSC2_EREFS = 1;		//oscillator is requested - important - set to 1
	ICSC2_ERCLKEN = 1;		//enables external ref clock for serclk
	
	//////////////////////////////////////////////////
	//ICSSC - Status and Control Register
	//DRS = 00
	//DMX32 - 0
	//the above gives FLL factor 512 and DCO range 16-20 mhz
	ICSSC_DRST_DRS1 = 0;
	ICSSC_DRST_DRS0 = 0;	
	ICSSC_DMX32 = 0;
}

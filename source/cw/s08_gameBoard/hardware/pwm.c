/*
 * pwm.c
 *
 *  Created on: Aug 17, 2019
 *      Author: danao
 *  
 *  The purpose of this file is to configure and
 *  control the PWM output on PC0.  The PWM output
 *  will be used to drive the speaker, changing the freq
 *  of the PWM to change the sound.  Configure the timer
 *  counter module for output compare, toggle on a counter
 *  match.  This gives duty cycle 50%.  Using the bus clock
 *  as the source clock, should be able to get 100hz to 20000hz.
 *  
 *  Note: SOPT2 bit 4 has to be set for output on TPM1CH2
 *  to output on correct pin, PC0
 */

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"
#include "pwm.h"


//////////////////////////////////////////////////
//Configure PWM output on PC0
//See Section 16.1 in the datasheet
//Configure output compare, output on PC0, toggle
//pin on counter match.
//PTCDD Bit 0 - No need to set as output
//freq should be between 100hz and 20000hz for prescale = 8
void PWM_init(unsigned long freq)
{
	unsigned long reloadValue = 0x00;
	
	reloadValue = (1000000 / (2*freq)) - 1;
	
	//TPM1SC - status and control register
	TPM1SC_TOIE = 0;		//no interrupt
	TPM1SC_CPWMS = 0;		//not center aligned
	
	//clock source
	TPM1SC_CLKSB = 0;		//clk source - 01 - bus clock
	TPM1SC_CLKSA = 1;		//clk source - 01 - bus clock
	
	//prescaler bits - 000 = prescale = 1
	//011 - prescale 8
	TPM1SC_PS2 = 0;
	TPM1SC_PS1 = 1;
	TPM1SC_PS0 = 1;
	
	//counter register - write any number to high or low
	//clears counter.  Global, so affects all channels
	TPM1CNTH = 0;
	TPM1CNTL = 0;

	//Set the reload value to trigger
	//the pin change at a given frequency.
	//
	//value = (bus clock / prescale / freg) - 1
	//
	//For Example: A pin change at 500hz
	//Value = (8000000 / 8 / 500) - 1
	//Value = 999
	//Frequencies should be able to divide into 1000000
	TPM1C2VH = (uint8_t)((reloadValue) >> 8);
	TPM1C2VL = (uint8_t)((reloadValue) % 0xFF);
	
	//Modulo registers - set same as reload value	
	TPM1MODH = (uint8_t)((reloadValue) >> 8);
	TPM1MODL = (uint8_t)((reloadValue) % 0xFF);
	
	//status and control register TPM1CnSC
	//Configure for output compare, toggle on a match, channel 2
	TPM1C2SC_CH2IE = 0;		//no interrupt

	//Mode select A and B - MS2B and MS2A
	//output compare - 01
	TPM1C2SC_MS2B = 0;		//output compare
	TPM1C2SC_MS2A = 1;		//output compare
	
	//Toggle output on channel match
	TPM1C2SC_ELS2B = 0;		//toggle on match
	TPM1C2SC_ELS2A = 1;		//toggle on match
	
	//turn the PWM off initially
	PWM_Disable();
}


/////////////////////////////////////////////
//PWM_setFrequency()
//Set range from 100 to 20000 hz.  If the value
//is out of range, it defaults to default freq.
void PWM_setFrequency(unsigned long freq)
{
	unsigned long reloadValue = 0x00;
	
	reloadValue = (1000000 / (2*freq)) - 1;
	
	//set the registers
	TPM1C2VH = (uint8_t)((reloadValue) >> 8);
	TPM1C2VL = (uint8_t)((reloadValue) % 0xFF);
	
	//Modulo registers - set same as reload value
	TPM1MODH = (uint8_t)((reloadValue) >> 8);
	TPM1MODL = (uint8_t)((reloadValue) % 0xFF);

}


////////////////////////////////////////
void PWM_setFreq_kHz(uint8_t far freq)
{
	unsigned long reloadValue = 0x00;
	unsigned long _freq = ((unsigned long)freq) * 1000;
	
	reloadValue = (1000000 / (2*_freq)) - 1;
	
	//set the registers
	TPM1C2VH = (uint8_t)((reloadValue) >> 8);
	TPM1C2VL = (uint8_t)((reloadValue) % 0xFF);
	
	//Modulo registers - set same as reload value
	TPM1MODH = (uint8_t)((reloadValue) >> 8);
	TPM1MODL = (uint8_t)((reloadValue) % 0xFF);	
}



//////////////////////////////////
//Enable the PWM output
void PWM_Enable(void)
{
	//clock source
	TPM1SC_CLKSB = 0;		//clk source - 01 - bus clock
	TPM1SC_CLKSA = 1;		//clk source - 01 - bus clock
}

/////////////////////////////////////
//Disable the clock source to PWM output
void PWM_Disable(void)
{
	//clock source
	TPM1SC_CLKSB = 0;		//clk source - 00 - none
	TPM1SC_CLKSA = 0;		//clk source - 00 - none
}

////////////////////////////////////
//Read the clock source bit.  If the 
//PWM output is enabled, the bit is high
uint8_t PWM_isEnabled(void)
{
	return TPM1SC_CLKSA;		//clock source A - 1 if it's on
}





//////////////////////////////////////////////////////////
//Timer 1 Overflow interrupt
//#define VectorNumber_Vtpm1ovf           7U
//#define VectorNumber_Vtpm1ch2           6U
//
//
//This interrupt occurred when the overflow flag
//was set.  Clear it by reading the status and 
//control register and setting the TOF flag to 0
void interrupt VectorNumber_Vtpm1ovf tpm1ovf_isr(void)
{
	uint8_t result = TPM1SC;		//read the register
	TPM1SC_TOF = 0;					//clear the flag
}


//Channel match interrupt.  Note:  I could not
//get this to work for center aligned PWM mode.
//I could get it to work for output compare
void interrupt VectorNumber_Vtpm1ch2 tpm1ch2_isr(void)
{
	uint8_t result = TPM1C2SC;		//read the register
	TPM1C2SC_CH2F = 0;				//clear the flag
}










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


unsigned long mPWMFrequency = 0x00;

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
	
	if ((freq > PWM_MAX_FREQ) || (freq < PWM_MIN_FREQ))
		mPWMFrequency = PWM_DEFAULT_FREQ;
	else
		mPWMFrequency = freq;
	
	reloadValue = (1000000 / (2*mPWMFrequency)) - 1;
	
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
}


/////////////////////////////////////////////
//PWM_setFrequency()
//Set range from 100 to 20000 hz.  If the value
//is out of range, it defaults to default freq.
void PWM_setFrequency(unsigned long freq)
{
	unsigned long reloadValue = 0x00;
	
	//disable interrupts
	DisableInterrupts;
	
	
	if ((freq < PWM_MIN_FREQ) || (freq > PWM_MAX_FREQ))
		mPWMFrequency = PWM_DEFAULT_FREQ;
	else
		mPWMFrequency = freq;

	reloadValue = (1000000 / (2*mPWMFrequency)) - 1;

	//reset the counter
//	TPM1CNTH = 0;
//	TPM1CNTL = 0;
	
	//set the registers
	TPM1C2VH = (uint8_t)((reloadValue) >> 8);
	TPM1C2VL = (uint8_t)((reloadValue) % 0xFF);
	
	//Modulo registers - set same as reload value
	TPM1MODH = (uint8_t)((reloadValue) >> 8);
	TPM1MODL = (uint8_t)((reloadValue) % 0xFF);
	
	//disable interrupts
	EnableInterrupts;
}


/////////////////////////////////////////////
//PWM_getFrequency
unsigned long PWM_getFrequency(void)
{
	return mPWMFrequency;
}



//////////////////////////////////////////////
//PWM_toggleFrequency
//Switches frequency from 1000hz to 2000hz
void PWM_toggleFrequency(void)
{
	if (mPWMFrequency == 2000)
	{
		mPWMFrequency = 1000;
		PWM_setFrequency(mPWMFrequency);
	}
	else
	{
		mPWMFrequency = 2000;
		PWM_setFrequency(mPWMFrequency);	
	}
}

/////////////////////////////////////////////
//Increment the frequency by PWM_FREQ_INCREMENET
void PWM_increaseFrequency(void)
{
	if (mPWMFrequency < (PWM_MAX_FREQ - PWM_FREQ_INCREMENT))
	{
		mPWMFrequency += PWM_FREQ_INCREMENT;
		PWM_setFrequency(mPWMFrequency);
	}
	else
	{
		mPWMFrequency = PWM_MIN_FREQ;
		PWM_setFrequency(mPWMFrequency);		
	}
}


/////////////////////////////////////////////
//Decrement the frequency by PWM_FREQ_INCREMENET
void PWM_decreaseFrequency(void)
{
	if (mPWMFrequency > (PWM_MIN_FREQ + PWM_FREQ_INCREMENT))
	{
		mPWMFrequency -= PWM_FREQ_INCREMENT;
		PWM_setFrequency(mPWMFrequency);
	}
	else
	{
		mPWMFrequency = PWM_MAX_FREQ;
		PWM_setFrequency(mPWMFrequency);		
	}	
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










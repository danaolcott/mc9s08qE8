/*
 * sound.c
 *
 *  Created on: Sep 18, 2019
 *      Author: danao
 * 
 * The purpose of this file is to create a sound scheme
 * that uses PWM to generate a frequency and a timer
 * for a duration.  
 */

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"
#include "sound.h"
#include "pwm.h"


///////////////////////////////////////////////////////
//Sound Beep
const uint8_t _soundBeep[] =
{4, 3, 2};

const SoundData soundBeep = {
	3,
	(uint8_t *far) _soundBeep,
};


///////////////////////////////////////////////
//Sound counters, etc
//static uint8_t mSoundCounter = 0x00;
static uint8_t *far mSoundPtr = (unsigned char *far)0x00;
//static uint8_t mSoundIndex = 0x00;

//areas in far memory
static uint8_t mSoundCounter @ 0x243u;
static uint8_t mSoundIndex @ 0x244u;





/////////////////////////////////////////////////////
void Sound_init(void)
{
	//turn off the PWM - disable the clock source
	PWM_Disable();
	mSoundCounter = 0x00;
	mSoundIndex = 0x00;	
	mSoundPtr = soundBeep.pSoundData;
}


////////////////////////////////////////////////////
//Play the sound associated with the beep
void Sound_playSound(void)
{
	unsigned long freq = 0x00;
	
	mSoundCounter = soundBeep.length;
	mSoundPtr = soundBeep.pSoundData;
	mSoundIndex = 0x00;
	
	//enable the PWM
	PWM_Enable();
	
	PWM_setFreq_kHz(mSoundPtr[mSoundIndex]);

	mSoundPtr++;
	mSoundCounter--;
	mSoundIndex++;
}


////////////////////////////////////////////////
//Called from the timer interrupt handler
//every 10th?, 100th? timeout.  Sound
//values assumed to play evenly
void Sound_InterruptHandler(void)
{	
	if (mSoundCounter > 0)
	{
		PWM_setFreq_kHz(mSoundPtr[mSoundIndex]);

		mSoundPtr++;
		mSoundIndex++;
		mSoundCounter--;
	}	
	else
	{
		//sound is over
		PWM_Disable();
		mSoundCounter = 0x00;
		mSoundIndex = 0x00;
	}		
}








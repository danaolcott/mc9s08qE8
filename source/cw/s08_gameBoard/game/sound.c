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
#include "rtc.h"


///////////////////////////////////////////////////////
//Sound Beep
const uint8_t _soundBeep[] =
{1, 2, 3, 4, 3, 2, 1};

const SoundData soundBeep = {
	7,
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


///////////////////////////////////////////
//Blocking methods for playing sound
void Sound_playPlayerFire_blocking(void)
{
	uint16_t i = 0x0;
	
	PWM_Enable();
	
	for (i = 2500 ; i > 1500 ; i-= 250)
	{
		PWM_setFrequency(i);
		RTC_delay(15);
	}		
	PWM_Disable();
}

void Sound_playEnemyFire_blocking(void)
{
	uint16_t i = 0x0;
	
	PWM_Enable();
	
	for (i = 1000 ; i < 2000 ; i+= 250)
	{
		PWM_setFrequency(i);
		RTC_delay(15);
	}		
	PWM_Disable();
}


void Sound_playPlayerExplode_blocking(void)
{	
	PWM_setFrequency(200);

	PWM_Enable();
	RTC_delay(100);
	PWM_Disable();
	RTC_delay(100);
	
	PWM_setFrequency(300);
	PWM_Enable();
	RTC_delay(200);
	PWM_Disable();
}

void Sound_playEnemyExplode_blocking(void)
{
	uint8_t i = 0x0;
	PWM_setFrequency(3000);	
	
	PWM_Enable();
	RTC_delay(80);
	PWM_Disable();
}

void Sound_playLevelUp_blocking(void)
{
	PWM_setFrequency(500);	
	PWM_Enable();
	RTC_delay(100);
	PWM_setFrequency(3000);	
	RTC_delay(50);
	PWM_Disable();
	RTC_delay(50);
	PWM_Enable();
	RTC_delay(50);
	PWM_Disable();
}

void Sound_playGameOver_blocking(void)
{
	uint8_t i = 0x0;
	PWM_setFreq_kHz(1);	
	
	for (i = 0x00 ; i < 8 ; i++)
	{
		PWM_Enable();
		RTC_delay(50);
		PWM_Disable();	
		RTC_delay(50);		
	}	
}









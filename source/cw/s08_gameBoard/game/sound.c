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
#include "gpio.h"





/////////////////////////////////////////////////////
void Sound_init(void)
{
	//turn off the PWM - disable the clock source
	PWM_Disable();
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
		RTC_delay(1);
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
		RTC_delay(1);
	}		
	PWM_Disable();
}


void Sound_playPlayerExplode_blocking(void)
{	
	PWM_setFrequency(200);

	PWM_Enable();
	RTC_delay(10);
	PWM_Disable();
	RTC_delay(10);
	
	PWM_setFrequency(300);
	PWM_Enable();
	RTC_delay(20);
	PWM_Disable();
}

void Sound_playEnemyExplode_blocking(void)
{
	uint8_t i = 0x0;
	PWM_setFrequency(3000);	
	
	PWM_Enable();
	GPIO_setRed();
	RTC_delay(8);
	GPIO_clearRed();
	PWM_Disable();
}

void Sound_playLevelUp_blocking(void)
{
	PWM_setFrequency(500);	
	PWM_Enable();
	GPIO_setRed();
	RTC_delay(10);
	GPIO_clearRed();
	PWM_setFrequency(3000);	
	RTC_delay(5);	
	PWM_Disable();
	RTC_delay(5);
	GPIO_setRed();
	PWM_Enable();
	RTC_delay(5);
	GPIO_clearRed();
	PWM_Disable();
}

void Sound_playGameOver_blocking(void)
{
	uint8_t i = 0x0;
	PWM_setFreq_kHz(1);	
	
	for (i = 0x00 ; i < 8 ; i++)
	{
		GPIO_setRed();
		PWM_Enable();
		RTC_delay(5);
		GPIO_clearRed();
		PWM_Disable();
		RTC_delay(5);		
	}	
}









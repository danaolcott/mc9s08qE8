/*
 * gpio.c
 *
 *  Created on: Aug 30, 2019
 *      Author: danao
 * 
 * The purpose of this file is to configure
 * gpio pins for the leds and user buttons
 * 
 * 
 */

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"
#include "gpio.h"
#include "pwm.h"
#include "game.h"		//button press flags
////////////////////////////////////////
//GPIO_init()
//Configure the red and green leds on PA6 and PA7
//Configure the user buttons on PA0, PB0, PB1
//as input.  Configure PB1 as interrupt (fire button)
void GPIO_init(void)
{
	//LEDs - PA6 and PA7
	PTADD |= BIT6;		//LED Red
	PTADD |= BIT7;		//LED Green

	PTAD &=~ BIT6;		//off
	PTAD &=~ BIT7;		//off
	
	//user buttons
	PTADD &=~ BIT0;		//User Button
	PTBDD &=~ BIT0;		//User Button
	PTBDD &=~ BIT1;		//User Button
	
	//////////////////////////////////////////////
	//PB1 - input, falling edged trigger interrupt
	//Registers:
	//KBISC - Status and control register
	//KBF - Bit 3 - KBI interrupt flag
	//KBACK - Bit 2 - acknowledge, reading is always 0, write a 1 to clear it
	//KBIE - bit 1 - interrupt enable, 1 = enabled.
	//KBIMOD - bit 0 - edge detection = 0 = edges only
	KBISC_KBACK = 1;			//clear any interrupt flags
	KBISC_KBIE = 0;				//disable interrupts temporarily
	KBISC_KBIMOD = 0;			//edge detection only
	
	//KBIPE - interrupt pin enable - PA0, PB0, PB1
	//See data sheet Figure 7-1
//	KBIPE_KBIPE0 = 1;			//enable PA0 interrupt
//	KBIPE_KBIPE4 = 1;			//enable PB0 interrupt
	KBIPE_KBIPE5 = 1;			//enable PB1 interrupt
		
	//KBIES - edge select register - falling or rising edge
	KBIES_KBEDG0 = 0;			//falling edge

	//enable interrupts
	KBISC_KBIE = 1;				//enable interrupts	
}



////////////////////////////////////////
//Toggle red LED
void GPIO_toggleRed(void){
	PTAD ^= BIT7;	
}

void GPIO_setRed(void){
	PTAD |= BIT7;	
}

void GPIO_clearRed(void){
	PTAD &=~ BIT7;	
}



//////////////////////////////////////////
//Toggle green LED
void GPIO_toggleGreen(void){
	PTAD ^= BIT6;
}

void GPIO_setGreen(void){
	PTAD |= BIT6;
}

void GPIO_clearGreen(void){
	PTAD &=~ BIT6;
}

 

//////////////////////////////////////////////////////////
//Keyboard Pin ISR
//see mc9s08qe8.h
//vector 18, maps to address: 0xFFDA
//
//PB1 - fire button, set the flag for player fire
void interrupt VectorNumber_Vkeyboard kbi_isr(void)
{
	KBISC_KBACK = 1;	//clear the interrupt flag

	//right
	if (!(PTBD & BIT1))
	{
		Game_flagSetButtonPress();
	}
}



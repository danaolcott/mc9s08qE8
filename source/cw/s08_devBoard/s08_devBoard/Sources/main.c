/*
 * MC9S08QE8 Development Board
 * Dana Olcott
 * 5/6/19
 * 
 * The purpose of this project is to test the pins on the 
 * MC9S08QE8 Development Board from Dana's Boatshop.com.
 * The project will run in several different configurations
 * each used to test the pins for a different peripheral.
 * 
 * Pins:
 * PC0 - PC7 - Not Used
 * 
 * PA4 and PA5 - Leave these alone - for debugging / reset 
 * PA0 - User Button
 * PA1 - Not Used
 * PA2 - Red
 * PA3 - Green
 * PA6 - Blue
 * PA7 - Yellow
 * 
 * PB0 - PB7 - Not used
 *  
 */


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "config.h"
#include "rtc.h"
#include "clock.h"
#include "spi.h"
#include "adc.h"
#include "uart.h"

#define MAX_FLASH_ROUTINE		10


//prototypes
void System_init(void);
void GPIO_init(void);

//globals
volatile char flashRoutine = 0x00;
uint8_t tx[4] = {0xAA, 0xCC, 0xAA, 0xCC};

void main(void)
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	Clock_init();				//configure clock for external
//	RTC_init_internal(RTC_FREQ_1000HZ);	//Timer
	RTC_init_external();		//configure as external	
	GPIO_init();				//IO	
	SPI_init();
	
	EnableInterrupts;			//enable interrupts
	
	while (1)
	{
		PTAD ^= BIT2;			//toggle an led
		SPI_writeArray(tx, 4);	//send something over SPI
		RTC_delay(500);			//wait
	}
}


////////////////////////////////////////////
//Configure system level registers - SOPT1 / SOPT2
//These are write one-time registers.  Changing
//any bit a second time has no effect.
//
//SOPT1 - Disable the watchdog and enable PTA5 as reset
void System_init(void)
{
	//SOPT1 Register - Default = 0xC2
	//Bit7 - Watchdog Enable
	//Bit0 - Enable PTA5 as reset
	SOPT1 = 0x43;
}



////////////////////////////////////////
//GPIO_init()
//Configure leds as output and user button as input
//
void GPIO_init(void)
{		
	//Port A
	PTADD &=~ BIT0;		//User Button
	PTADD |= BIT1;
	PTADD |= BIT2;		//LED Red
	PTADD |= BIT3;		//LED Green
	PTADD |= BIT6;		//LED Blue
	PTADD |= BIT7;		//LED Yellow
		
	//////////////////////////////////////////////
	//PA0 - input, falling edged trigger interrupt
	//Registers:
	//KBISC - Status and control register
	//KBF - Bit 3 - KBI interrupt flag
	//KBACK - Bit 2 - acknowledge, reading is always 0, write a 1 to clear it
	//KBIE - bit 1 - interrupt enable, 1 = enabled.
	//KBIMOD - bit 0 - edge detection = 0 = edges only
	KBISC_KBACK = 1;			//clear any interrupt flags
	KBISC_KBIE = 0;				//disable interrupts temporarily
	KBISC_KBIMOD = 0;			//edge detection only
	
	//KBIPE - interrupt pin enable
	KBIPE_KBIPE0 = 1;			//enable PA0 interrupt
	
	//KBIES - edge select register - falling or rising edge
	KBIES_KBEDG0 = 0;			//falling edge

	//enable interrupts
	KBISC_KBIE = 1;				//enable interrupts	
}





//////////////////////////////////////////////////////////
//Keyboard Pin ISR
//see mc9s08qe8.h
//vector 18, maps to address: 0xFFDA
void interrupt VectorNumber_Vkeyboard kbi_isr(void)
{
	KBISC_KBACK = 1;	//clear the interrupt flag

	//do something	
	PTAD ^= BIT6;
}





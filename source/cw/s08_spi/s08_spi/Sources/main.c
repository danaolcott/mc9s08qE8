/*
 * MC9S08QE8 Blink Example
 * Dana Olcott
 * 4/11/19
 * 
 * This project configures the MC9S08QE8
 * for the following:
 * 
 * LEDs: 
 * PTC0 - output - LED Red
 * PTC1 - output - LED Green
 * 
 * User Button - PA0 - Pin 26, 10k pullup
 * Uses keyboard interrupt, P0 - KBIP0
 * 
 * 
 * RTC Timer configured to generate an 
 * interrupt at a specified rate based on the
 * 1khz low power clock source.  See rtc.c / rtc.h
 *  
 */


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include <stddef.h>
#include "config.h"
#include "rtc.h"
#include "spi.h"

//prototypes
void System_init(void);
void GPIO_init(void);

void LED_Toggle_Red(void);
void LED_Toggle_Green(void);

unsigned char tx[3] = {0xAA, 0xBB, 0xCE};


void main(void) 
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	RTC_init(RTC_FREQ_100HZ);	//Timer
	GPIO_init();				//IO
	SPI_init();
	EnableInterrupts;			//enable interrupts
	
	while (1) 
	{
		LED_Toggle_Red();
		RTC_delay(10);		
		SPI_writeArray(tx, 3);
		SPI_write(0xAA);
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
//PTC0 and PTC1 - Outputs - LEDS Red and Green
//PA0 - Keyboard interrupt, P0 - KBIP0
//configured as falling edge trigger
//
void GPIO_init(void)
{	
	//LEDs red and green as output
	PTCDD |= BIT0;
	PTCDD |= BIT1;
	
	PTCD &=~ BIT0;
	PTCD &=~ BIT1;
	
	
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



////////////////////////////////////////
//PTC0
void LED_Toggle_Red(void)
{
	PTCD ^= BIT0;
}


////////////////////////////////////////
//PTC1
void LED_Toggle_Green(void)
{
	PTCD ^= BIT1;
}



//////////////////////////////////////////////////////////
//Keyboard Pin ISR
//see mc9s08qe8.h
//vector 18, maps to address: 0xFFDA
void interrupt VectorNumber_Vkeyboard kbi_isr(void)
{
	KBISC_KBACK = 1;	//clear the interrupt flag
	
	LED_Toggle_Green();
	
}




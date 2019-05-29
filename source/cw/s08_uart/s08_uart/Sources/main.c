/*
 * MC9S08QE8 UART Example
 * Dana Olcott
 * 5/6/19
 * 
 * The purpose of this project is to configure the UART
 * peripheral on the MC9S08QE8.
 * 
 * Pinout:
 * UART TX - PTB1 - TX
 * UART RX - PTB0 - RX
 * 
 * Note that these are shared with keyboard interrupt
 * channels 4 and 5, and adc channels 4 and 5.
 * 
 * This project will read the temp sensor on the 
 * IC and output the data over the serial port.
 * 
 * Other peripherals supporting the project include:
 * 
 * LEDs: 
 * PTC0 - output - LED Red
 * PTC1 - output - LED Green
 * 
 * User Button - PA0 - Pin 26, 10k pullup
 * Uses keyboard interrupt, P0 - KBIP0
 * 
 * RTC Timer configured to generate an 
 * interrupt at a specified rate based on the
 * 1khz low power clock source.  See rtc.c / rtc.h
 * 
 * SPI peripheral used to output the values read by
 * the ADC.  SPI Pins: PB2 - PB5
 * 
 *  PB2 - SCK		Pin 18
 *  PB3 - MOSI		Pin 17
 *  PB4 - MISO		Pin 12
 *  PB5 - SS - 		Pin 11 - Configure as normal IO
 *  
 * 
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
#include "spi.h"
#include "adc.h"
#include "uart.h"

//prototypes
void System_init(void);
void GPIO_init(void);

void LED_Toggle_Red(void);
void LED_Toggle_Green(void);
void LED_On_Red(void);
void LED_On_Green(void);
void LED_Off_Red(void);
void LED_Off_Green(void);

uint16_t result = 0x00;
uint8_t high = 0x00;
uint8_t low = 0x00;
int16_t temperature = 0x00;
int n = 0x00;
unsigned char outBuffer[64];

void main(void) 
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	RTC_init(RTC_FREQ_100HZ);	//Timer
	GPIO_init();				//IO
	ADC_init();					//set up ADC on CH8 and CH9
	UART_init(BAUD_RATE_19200);	//setup uart on PB1 and PB0
	
	EnableInterrupts;			//enable interrupts
	
	while (1)
	{
		LED_Toggle_Red();

		//read the temp, 10ths of a degree
		temperature = ADC_readTemp();
		n = sprintf(outBuffer, "Temperature: %d.%d\r\n", temperature / 10, temperature % 10);
		UART_sendStringLength(outBuffer, n);
		
		RTC_delay(200);
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

void LED_On_Red(void)
{
	PTCD |= BIT0;
}

void LED_On_Green(void)
{
	PTCD |= BIT1;
}

void LED_Off_Red(void)
{
	PTCD &=~ BIT0;
}

void LED_Off_Green(void)
{
	PTCD &=~ BIT1;
}





//////////////////////////////////////////////////////////
//Keyboard Pin ISR
//see mc9s08qe8.h
//vector 18, maps to address: 0xFFDA
void interrupt VectorNumber_Vkeyboard kbi_isr(void)
{
	KBISC_KBACK = 1;	//clear the interrupt flag
	
	//increment the toggle interval
	if (mToggleInterval < 10)
		mToggleInterval++;
	else
		mToggleInterval = 1;
	
	
	LED_Toggle_Red();
	
}




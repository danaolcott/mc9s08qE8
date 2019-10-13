/*
 * I2C Project using the NXP Development Board from
 * Dana's Boatshop.com.
 * 
 * The purpose of this project is to make the i2c
 * peripheral communicate with a simple memory chip.
 * Assume it will run in master mode with tx/rx enabled.
 * 
 * The board uses the following peripherals:
 * RTC - use internal 1000hz clock for timeout
 * Bus clock - use the external 16mhz xtal. - PB6 and PB7
 * 
 * PA0 - user button
 * PA2 and PA3 - I2C (remove R2 and R3 - LEDs red and green)
 * PA6 - LED Blue
 * PA7 - LED Yellow
 * 
 * 
 * 
 */


#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "config.h"
#include "main.h"
#include "rtc.h"
#include "clock.h"
#include "i2c.h"


void System_init(void);
void GPIO_init(void);
//void LED_toggleBlue(void);
//void LED_toggleOrange(void);

uint8_t counter = 0x00;
uint8_t status = 0x00;
uint8_t tx[4] = {0x00};
uint8_t rx[16] = {0x00};

void main(void)
{
	DisableInterrupts;
	
	//Configure the hardware
	System_init();
	GPIO_init();
	Clock_init();							//16mhz bus clock
	RTC_init_internal(RTC_FREQ_1000HZ);		//rtc runs on the internal 1000hz
//	RTC_init_external();					//rtc runs on the external bus clock
	I2C_init();
	
	EnableInterrupts;
	
	while(1)
	{
		LED_toggleOrange();

		/*
		//load the array
		tx[0] = 0x81;
		tx[1] = counter++;
		tx[2] = 0x81;		
		
		status = I2C_writeData(I2C_ADDRESS, tx, 3);
		
		if (status == IIC_READY_STATUS)
			LED_toggleBlue();
		
		//light sensor - command, data
		//1110 0000  0x03 - 
	*/
//		status = I2C_readData(I2C_ADDRESS, rx, 1);

//		uint8_t I2C_writeReadData(uint8_t address, uint8_t far* txData, uint8_t txBytes, uint8_t far* rxData, uint8_t rxBytes)

		
		//try a block read starting at address 0
		// 1101 0000 = 0xD0, 1
		tx[0] = 0xAA;
		
		//single byte read at address 0x0A - returns 0x50
		//1100 1010
		tx[0] = 0x81;
		tx[1] = 0xAA;
		tx[2] = 0xAA;
				
		//TODO: Fix the counts...
//		status = I2C_writeReadData(I2C_ADDRESS, tx, 1, rx, 7);
//		status = I2C_memoryRead(I2C_ADDRESS, 0xCA, 1, rx, 1);
		status = I2C_memoryWrite(I2C_ADDRESS, 0xCA, 1, tx, 1);
		

		if (status == IIC_ERROR_STATUS)
			LED_toggleBlue();

		RTC_delay(100);
	}
}


void System_init(void)
{
	//SOPT1 Register - Default = 0xC2
	//Bit7 - Watchdog Enable
	//Bit0 - Enable PTA5 as reset
	SOPT1 = 0x43;
}


void GPIO_init(void)
{
	//Port A
	PTADD &=~ BIT0;		//User Button
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



void LED_toggleBlue(void)
{
	PTAD ^= BIT6;
}

void LED_toggleOrange(void)
{
	PTAD ^= BIT7;	
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





/*
 * MC9S08QE8 Development Board - Gamer's Edition
 * Dana Olcott
 * 8/11/19
 * 
 * The purpose of this project is to configure all the 
 * GPIO, peripherals, etc for the game board.  Schematics are 
 * shown in the schematics folder in github.  The board uses the
 * following pinout:
 * 
 * PA6 - LED Red
 * PA7 - LED Green
 * 
 * PA0 - User Button 1
 * PB0 - User Button 2
 * PB1 - User Button 3
 * 
 * PA1 - ADC Temperature Sensor
 * PC0 - PWM Output for Speaker / Sound
 * 
 * PB6 - XTAL Oscillator
 * PB7 - XTAL Oscillator
 * 
 * LCD Display
 * PB2 - SPI SCK
 * PB3 - SPI MOSI
 * PB4 - Not Used, but configure as SPI MISO
 * PB5 - SPI CS 
 * 
 * PC6 - LCD_RST
 * PC7 - LCD_CD
 * 
 * I2C Memory
 * PA2 - I2C SDA
 * PA3 - I2C SCL
 * 
 * Memory Allocation:
 * Use the tiny memory model, so that variables assigned
 * starting at 0x60 to 0xFF
 * 
 * Frame buffer assigned at 0x100 and size = 320 bytes
 * Remaining 32 bytes available starting at 0x240
 * 
 * 
 *  
 * 
 */
#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */

#include "mc9s08qe8.h"
//#include <stddef.h>
//#include <string.h>
//#include <stdio.h>
#include "config.h"
#include "rtc.h"
#include "clock.h"
#include "gpio.h"
#include "spi.h"
#include "adc.h"
#include "pwm.h"
#include "lcd.h"
#include "game.h"

//prototypes
void System_init(void);


volatile uint16_t adcResult = 0x00;
int i = 0;
int n = 0x0;
int counter = 0x00;
uint8_t playerPosition = 0x00;
uint8_t movingRight = 1;

char far buffer[16];

//overall, arrays should be assigned to a memory address
//directly and not rely on the compiler to do it for you
//because in this tiny memory model, you have no idea if 
//something is going to get corrupted
//
//uint8_t far msg2[10];		//this works - assign msg2 to far section of RAM
//uint8_t near msg2[10];		//this works - assign msg2 to zero page, near keyword
//uint8_t msg2[10];			//this does not work - garbage
//
//Passing pointer into function - dont try it.



void main(void) 
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	Clock_init();				//configure clock for external
	RTC_init_internal(RTC_FREQ_1000HZ);
	
	GPIO_init();				//IO
	ADC_init();					//ADC Channel 1 - temp sensor
	PWM_init(100);				//PWM output on PC0
	SPI_init();					//configure the SPI
	LCD_init();					//configure the LCD

	//set up the LCD
	LCD_clear(0x00);			//clear screen
	LCD_clearBackground(0xAA);	//margins
	
	Game_init();				//initialize the game
	
	EnableInterrupts;			//enable interrupts


	
	
	
	while (1)
	{	
		//check flag missile launch
		if (!(counter % 10))
		{
			Game_missileEnemyLaunch();
		}
		
		//check flag enemy hit
		
		//check flag player hit
		if (Game_flagGetPlayerHitFlag() == GAME_FLAG_PLAYER_HIT)
		{
			Game_flagClearPlayerHitFlag();
//			DisableInterrupts;
//			Game_playExplosionPlayer();	
//			EnableInterrupts;
		}
		
		//TODO: update left and right buttons as polling
		
		//left and right buttons - dont
		
		//check flag button press - move left
		if (Game_flagGetButtonPress() == BUTTON_LEFT)
		{
			Game_flagClearButtonPress(BUTTON_LEFT);
			Game_playerMoveLeft();
		}

		//check flag button press - move right
		if (Game_flagGetButtonPress() == BUTTON_RIGHT)
		{
			Game_flagClearButtonPress(BUTTON_RIGHT);
			Game_playerMoveRight();
		}
		
		//check flag button press - fire
		if (Game_flagGetButtonPress() == BUTTON_FIRE)
		{
			Game_flagClearButtonPress(BUTTON_FIRE);
//			Game_missilePlayerLaunch();
		}
		
		
		
//		Game_playerMoveDemo();					//player flag
		Game_enemyMove();					//move enemy
		Game_missileMove();					//move all missiles

		DisableInterrupts;					//stop the timer
		LCD_clearFrameBuffer(0, 0);			//clear the ram buffer
		Game_playerDraw();					//update player image
		Game_enemyDraw();					//draw enemy
		Game_missileDraw();					//draw missiles
		LCD_updateFrameBuffer();			//update the display
		EnableInterrupts;

		counter++;
		GPIO_toggleGreen();
		RTC_delay(100);
		
	}
}


////////////////////////////////////////////
//Configure system level registers - SOPT1 / SOPT2
//These are write one-time registers.  Changing
//any bit a second time has no effect.
//
//SOPT1 - Disable the watchdog and enable PTA5 as reset

//SOPT2 - The appropriate bit to configure PWM output
//		  on PC0 for TPM1CH2 - set to 1, default is 0
void System_init(void)
{
	//SOPT1 Register - Default = 0xC2
	//Bit7 - Watchdog Enable
	//Bit1 - 1 to enable BKGD/MS mode
	//Bit0 - Enable PTA5 as reset
	SOPT1 = 0x43;

	//this should disable all ability to be in debug mode
//	SOPT1 = 0x41;		//bit 1 - set to 0 for BKGD/MS as PTA4

	//SOPT2 - Section 5.8.5 in datasheet - Bit 4 set high
	//for TPM1CH2 output on PC0
	//default value for the register is 0x00
	SOPT2 = 0x10;
}








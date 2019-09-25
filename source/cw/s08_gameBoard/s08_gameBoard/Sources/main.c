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
#include "config.h"
#include "rtc.h"
#include "clock.h"
#include "gpio.h"
#include "spi.h"
#include "adc.h"
#include "pwm.h"
#include "lcd.h"
#include "game.h"
#include "sound.h"

//prototypes
void System_init(void);

//variables in main.
static unsigned int gameLoopCounter = 0x00;
uint8_t length = 0x00;
static uint8_t far printBuffer[GAME_PRINT_BUFFER_SIZE] = {0x00};


void main(void) 
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	Clock_init();				//configure clock for external
	RTC_init_internal(RTC_FREQ_1000HZ);	//use internal timer for 1000hz interrupt
	
	GPIO_init();				//IO
	ADC_init();					//ADC Channel 1 - temp sensor
	PWM_init(1000);				//PWM output on PC0
	SPI_init();					//configure the SPI
	LCD_init();					//configure the LCD	
	Game_init();				//initialize the game
	Sound_init();
	EnableInterrupts;			//enable interrupts
		
	while (1)
	{	
		//check for player move - move left
		if (!(PTAD & BIT0))
			Game_playerMoveLeft();
		
		//check for player move - move right
		if (!(PTBD & BIT0))
			Game_playerMoveRight();
				
		//check flag button press - fire
		if (Game_flagGetButtonPress() == 1)
		{
			Game_flagClearButtonPress();
			Game_missilePlayerLaunch();
			Sound_playPlayerFire_blocking();
		}

		//check flag enemy missile launch
		if (!(gameLoopCounter % 10))
		{
			Game_missileEnemyLaunch();
			Sound_playEnemyFire_blocking();
		}
		
		//check flag player hit
		if (Game_flagGetPlayerHitFlag() == 1)
		{
			Game_flagClearPlayerHitFlag();
			Game_playExplosionPlayer_withSound();
		}
		
		//check flag - enemy hit flag
		if (Game_flagGetEnemyHitFlag() == 1)
		{
			Game_flagClearEnemyHitFlag();
			Sound_playEnemyExplode_blocking();
		}
		
		//check level up flag
		if (Game_flagGetLevelUpFlag() == 1)
		{
			Game_flagClearLevelUpFlag();
			Sound_playLevelUp_blocking();
		}
		
		//check flag - game over
		if (Game_flagGetGameOverFlag() == 1)
		{
			Sound_playGameOver_blocking();

			while (Game_flagGetGameOverFlag() == 1)
			{
				Game_playGameOver();
				
				//if either left or right
				if ((!(PTAD & BIT0)) || (!(PTBD & BIT0)))
				{
					Game_flagClearGameOverFlag();
					
					DisableInterrupts;
					Game_init();
					EnableInterrupts;
				}

				GPIO_toggleRed();
				RTC_delay(500);				
			}
		}
		

		//move enemy and missile				
		Game_enemyMove();					//move enemy
		Game_missileMove();					//move all missiles

		//update display with interrupts disabled
		DisableInterrupts;					//stop the timer
		LCD_clearFrameBuffer(0, 0);			//clear the ram buffer
		Game_playerDraw();					//update player image
		Game_enemyDraw();					//draw enemy
		Game_missileDraw();					//draw missiles
		LCD_updateFrameBuffer();			//update the display
		
		//display the header info
		LCD_drawString(0, 0, "S:");
		//draw the score, level, etc

		length = LCD_decimalToBuffer(Game_getGameScore(), printBuffer, (uint8_t)GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(0, 18, printBuffer, length);

		LCD_drawString(0, 60, "L:");
		length = LCD_decimalToBuffer(Game_getGameLevel(), printBuffer, (uint8_t)GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(0, 74, printBuffer, length);
		
		//display number of players as image
		switch(Game_getNumPlayers())
		{
			case 3:	LCD_drawImagePage(0, 90, BITMAP_PLAYER_ICON3);	break;
			case 2:	LCD_drawImagePage(0, 90, BITMAP_PLAYER_ICON2);	break;
			case 1:	LCD_drawImagePage(0, 90, BITMAP_PLAYER_ICON1);	break;
		}

		EnableInterrupts;

		gameLoopCounter++;
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








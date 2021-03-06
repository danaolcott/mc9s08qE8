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
 * 7947
 * 527
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
#include "i2c.h"
#include "eeprom.h"
#include "pwm.h"
#include "lcd.h"
#include "game.h"
#include "sound.h"

//prototypes
void System_init(void);

//variables in main.
static unsigned int gameLoopCounter = 0x00;
uint8_t length = 0x00;
static char far printBuffer[GAME_PRINT_BUFFER_SIZE] = {0x00};
uint16_t cycleCounter = 0x00;
uint8_t launchResult = 0x00;
uint8_t missileFlag = 0x00;

void main(void) 
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	Clock_init();				//configure clock for external
	RTC_init_internal(RTC_FREQ_100HZ);	//use internal timer for 100hz interrupt
	
	GPIO_init();				//IO
	PWM_init(1000);				//PWM output on PC0
	SPI_init();					//configure the SPI
	I2C_init();					//configure i2c on PA2 and PA3
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
			launchResult = Game_missilePlayerLaunch();
			if (launchResult == 1)
				Sound_playPlayerFire_blocking();
		}

		//check flag enemy missile launch
		if (!(gameLoopCounter % 10))
		{
			launchResult = Game_missileEnemyLaunch();			
			if (launchResult == 1)
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
			Game_flagClearLevelUpFlag();	//clear the flag
			Game_levelUp();					//level up			
			Sound_playLevelUp_blocking();	//play sound
		}
		
		//check flag - game over
		if (Game_flagGetGameOverFlag() == 1)
		{
			Sound_playGameOver_blocking();
			
			//update the cycle counter - pass 0 as the
			//clear flag
			cycleCounter = EEPROM_updateCycleCount();

			while (Game_flagGetGameOverFlag() == 1)
			{
				Game_playGameOver();
				
				//draw the new cycle counter
				LCD_drawString(1, 0, "Game#:");
				length = LCD_decimalToBuffer(cycleCounter, printBuffer, GAME_PRINT_BUFFER_SIZE);
				LCD_drawStringLength(1, 50, printBuffer, length);
				
				//if either left or right
				if ((!(PTAD & BIT0)) || (!(PTBD & BIT0)))
				{
					Game_flagClearGameOverFlag();
					
					DisableInterrupts;
					Game_init();
					EnableInterrupts;
				}

				GPIO_toggleRed();
				RTC_delay(50);				
			}
		}
		
		//move enemy and missile				
		Game_enemyMove();					//move enemy
		missileFlag = Game_missileMove();	//move all missiles

		//update display with interrupts disabled
		DisableInterrupts;					//stop the timer
		LCD_clearFrameBuffer(0, 0);			//clear the ram buffer
		Game_playerDraw();					//update player image
		Game_enemyDraw();					//draw enemy
		Game_missileDraw();					//draw missiles
		LCD_updateFrameBuffer();			//update the display
		
		//display the header info - score, level, num players
		LCD_drawString(0, 0, "S:");
		length = LCD_decimalToBuffer(Game_getGameScore(), printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(0, 18, printBuffer, length);

		LCD_drawString(0, 60, "L:");
		length = LCD_decimalToBuffer(Game_getGameLevel(), printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(0, 74, printBuffer, length);
		
		switch(Game_getNumPlayers())
		{
			case 3:	LCD_drawImagePage(0, 90, BITMAP_PLAYER_ICON3);	break;
			case 2:	LCD_drawImagePage(0, 90, BITMAP_PLAYER_ICON2);	break;
			case 1:	LCD_drawImagePage(0, 90, BITMAP_PLAYER_ICON1);	break;
		}

		//reenable interrupts
		EnableInterrupts;

		gameLoopCounter++;
		GPIO_toggleGreen();	//170ms with RTC at 1000hz, 155ms at 100hz
		RTC_delay(10);
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








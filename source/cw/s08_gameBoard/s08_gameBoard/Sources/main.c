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
#include "i2c.h"
#include "eeprom.h"
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
static char far printBuffer[GAME_PRINT_BUFFER_SIZE] = {0x00};
uint8_t counter = 0x00;
uint8_t value = 0x00;
uint8_t i = 0x00;


void main(void) 
{
	DisableInterrupts;			//disable interrupts
	System_init();				//configure system level config bits
	Clock_init();				//configure clock for external
	RTC_init_internal(RTC_FREQ_100HZ);	//use internal timer for 100hz interrupt
	
	GPIO_init();				//IO
	ADC_init();					//ADC Channel 1 - temp sensor
	PWM_init(1000);				//PWM output on PC0
	SPI_init();					//configure the SPI
	I2C_init();					//configure i2c on PA2 and PA3
	EEPROM_init();				//init the memory ic
	LCD_init();					//configure the LCD	
	Game_init();				//initialize the game
	Sound_init();
	EnableInterrupts;			//enable interrupts
	
	//eeprom test
	while(1)
	{
		//write value to address
		//EEPROM_writeByte(counter, counter);
		
		RTC_delay(10);
		
		//TODO: Fix the read.  Its returning the read
		//address, not the value in memory.  ie,
		//the memory i2c address is (0x50 << 1) with LSB high, for 0xA1
		//it's returning 161 (0xA1)
		
		//read it back
		value = EEPROM_readByte(counter);
		
		//write value and counter to display
		LCD_clearPage(0, 0);
		LCD_clearPage(1, 0);
		LCD_clearPage(2, 0);
		LCD_clearPage(3, 0);
		
		
		//print the value on the lcd
		LCD_drawString(0, 0, "C:");
		length = LCD_decimalToBuffer(counter, printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(0, 14, printBuffer, length);
		
		LCD_drawString(0, 50, "V:");
		length = LCD_decimalToBuffer(value, printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(0, 64, printBuffer, length);
		
		LCD_drawString(1, 0, "Rx0:");
		length = LCD_decimalToBuffer(I2C_RX_DATA[0], printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(1, 32, printBuffer, length);

		LCD_drawString(2, 0, "Rx1:");
		length = LCD_decimalToBuffer(I2C_RX_DATA[1], printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(2, 32, printBuffer, length);

		LCD_drawString(3, 0, "Rx2:");
		length = LCD_decimalToBuffer(I2C_RX_DATA[2], printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(3, 32, printBuffer, length);
		
		LCD_drawString(4, 0, "Rx3:");
		length = LCD_decimalToBuffer(I2C_RX_DATA[3], printBuffer, GAME_PRINT_BUFFER_SIZE);
		LCD_drawStringLength(4, 32, printBuffer, length);
		
		
		if (value == counter)
		{
			GPIO_setGreen();
			GPIO_clearRed();
		}

		else
		{
			GPIO_setRed();
			GPIO_clearGreen();			
		}
		
		if (counter < 120)
			counter++;
		else
			counter = 0;
		
		RTC_delay(10);
		
		GPIO_clearGreen();
		GPIO_clearRed();

		RTC_delay(10);
						
	}
		
	/*
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
				RTC_delay(50);				
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
	
	*/
	
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








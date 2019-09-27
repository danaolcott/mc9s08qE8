/*
 * game.c
 *
 *  Created on: Sep 8, 2019
 *      Author: danao
 * 
 * Configuration file for the game.  Contains instances of 
 * the player, enemy, and missile arrays, all functions for
 * movement, launching missiles, etc.  
 * 
 */
#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include <stdlib.h>
#include "config.h"
#include "game.h"
#include "lcd.h"
#include "bitmap.h"
#include "gpio.h"
#include "rtc.h"		//delay
#include "pwm.h"

//Game objects
//Note: Declare as static and init to 0x00 to 
//get compiler to put into .bss.  Otherwise, it
//ends up in .common.  Erroneous results happen
//when it ends up in .common.

static PlayerStruct mPlayer = {0x00};
static EnemyStruct mEnemy[GAME_ENEMY_NUM_ENEMY] = {0x00};

//Missile arrays, assume the player and the enemy each get 4
static MissileStruct mPlayerMissile[GAME_MISSILE_NUM_MISSILE] = {0x00};
static MissileStruct mEnemyMissile[GAME_MISSILE_NUM_MISSILE] = {0x00};

//flags
volatile uint8_t mButtonFlag = 0x00;
volatile uint8_t mPlayerHitFlag = 0x00;
volatile uint8_t mGameOverFlag = 0x00;
volatile uint8_t mGameLevelUpFlag = 0x00;
volatile uint8_t mEnemyHitFlag = 0x00;

//areas in far memory
static uint16_t mGameScore @ 0x240u;
static uint8_t mGameLevel @ 0x242u;


void Game_init(void)
{
	//reset the score, flags, etc
	mButtonFlag = 0x00;
	mPlayerHitFlag = 0x00;
	mGameLevelUpFlag = 0x00;
	mEnemyHitFlag = 0x00;
	mGameScore = 0x00;
	mGameLevel = 0x00;
		
	LCD_clear(0x00);			//clear screen
	LCD_clearBackground(0xAA);	//margins

	Game_playerInit();
	Game_enemyInit();
	Game_missileInit();
	
	Game_playerDraw();
	Game_enemyDraw();
	
	//update the display contents
	LCD_updateFrameBuffer();
}

///////////////////////////////////////////////
//Player_init
//Reset the num lives and set initial position
void Game_playerInit(void)
{
	mPlayer.numLives = GAME_PLAYER_NUM_LIVES;
	mPlayer.xPosition = GAME_PLAYER_DEFAULT_X;
}

//////////////////////////////////////////////
//Enemy_init
//
void Game_enemyInit(void)
{
	uint16_t i, j = 0;
	uint8_t index = 0;

	for (i = 0 ; i < GAME_ENEMY_NUM_ROWS ; i++)
	{
		for (j = 0 ; j < GAME_ENEMY_NUM_COLS ; j++)
		{
			mEnemy[index].flag_VHL = 0x07;			//down, right, alive
			mEnemy[index].xPosition = (uint8_t)(GAME_ENEMY_X_SPACING * j);
			mEnemy[index].yPosition = (uint8_t)(GAME_ENEMY_Y_SPACING * i);

			index++;
		}
	}
}


///////////////////////////////////////////////
//Missile_init
void Game_missileInit(void)
{
	uint8_t i = 0;
	
	for (i = 0 ; i < GAME_MISSILE_NUM_MISSILE ; i++)
	{
		mEnemyMissile[i].alive = 0;
		mEnemyMissile[i].x = 0x00;
		mEnemyMissile[i].y = 0x00;
		
		mPlayerMissile[i].alive = 0x00;
		mPlayerMissile[i].x = 0x00;
		mPlayerMissile[i].y = 0x00;
	}
}


void Game_playerMoveLeft(void)
{
	if (mPlayer.xPosition > GAME_PLAYER_MIN_X + 2)
		mPlayer.xPosition-=2;	
}

void Game_playerMoveRight(void)
{
	if (mPlayer.xPosition < GAME_PLAYER_MAX_X - 2)
		mPlayer.xPosition+=2;
}


///////////////////////////////////////////////
//Game_playerMove
//For now, just move the player left and right
void Game_playerMoveDemo(void)
{
	static uint8_t moveRight = 1;
	
	if (moveRight == 1)
	{
		if (mPlayer.xPosition < GAME_PLAYER_MAX_X)
			mPlayer.xPosition++;
		else
		{
			moveRight = 0;
			mPlayer.xPosition--;
		}
	}
	else
	{
		if (mPlayer.xPosition > GAME_PLAYER_MIN_X)
			mPlayer.xPosition--;
		else
		{
			moveRight = 1;
			mPlayer.xPosition++;
		}		
	}
}


///////////////////////////////////////////////
//Move enemies left and right, up and down
//
void Game_enemyMove(void)
{
	uint8_t i, j = 0;
	uint8_t flag = 0x00;
	
	//sizes
	uint8_t sizeX = bmenemy1Bmp.xSize;
	uint8_t sizeY = bmenemy1Bmp.ySize;

	//direction flags
	uint8_t right = 0x00;
	uint8_t down = 0x00;
	uint8_t alive = 0x00;
	
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		//get flags
		alive = mEnemy[i].flag_VHL & 0x01;
		right = (mEnemy[i].flag_VHL & 0x02) >> 1;
		down = (mEnemy[i].flag_VHL & 0x04) >> 2;

		//moving right
		if (right == 1)
		{
			if (((mEnemy[i].xPosition + sizeX) < GAME_ENEMY_MAX_X) && (alive == 1))
				mEnemy[i].xPosition += 2;

		}
		
		//moving left
		else
		{
			if ((mEnemy[i].xPosition > GAME_ENEMY_MIN_X) && (alive == 1))
				mEnemy[i].xPosition -= 2;
		}
	}
	
	//direction change - left
	flag = 0x00;	
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		//get flags
		alive = mEnemy[i].flag_VHL & 0x01;
		right = (mEnemy[i].flag_VHL & 0x02) >> 1;
		down = (mEnemy[i].flag_VHL & 0x04) >> 2;
		
		if (((mEnemy[i].xPosition + sizeX) >= GAME_ENEMY_MAX_X) && (alive == 1))
		{
			flag = 1;
			i = GAME_ENEMY_NUM_ENEMY;
			break;
		}
	}
	
	if (flag == 1)
	{
		//change the direction bit for each enemy - clear it for left
		for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
		{
			mEnemy[i].flag_VHL &=~ BIT1;
		}		
	}
	
	//direction change - right
	flag = 0x00;	
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		//read the flags
		alive = mEnemy[i].flag_VHL & 0x01;
		right = (mEnemy[i].flag_VHL & 0x02) >> 1;
		down = (mEnemy[i].flag_VHL & 0x04) >> 2;
		
		if ((mEnemy[i].xPosition <= GAME_ENEMY_MIN_X) && (alive == 1))
		{
			flag = 1;
			i = GAME_ENEMY_NUM_ENEMY;
			break;
		}
	}
	
	if (flag == 1)
	{
		//flip direction to right
		for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
		{
			mEnemy[i].flag_VHL |= BIT1;			//set direction right

			alive = mEnemy[i].flag_VHL & 0x01;
			right = (mEnemy[i].flag_VHL & 0x02) >> 1;
			down = (mEnemy[i].flag_VHL & 0x04) >> 2;

			//enemy are moving down
			if (down == 1)
			{
				//move down
				if(((mEnemy[i].yPosition + sizeY) < GAME_ENEMY_MAX_Y) && (alive == 1))
					mEnemy[i].yPosition++;
			}
			
			//enemy are moving up
			else
			{
				if((mEnemy[i].yPosition > GAME_ENEMY_MIN_Y) && (alive == 1))
					mEnemy[i].yPosition--;
			}
		}
	}
	
	//check for direction change up
	flag = 0;
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		//read the flags
		alive = mEnemy[i].flag_VHL & 0x01;
		right = (mEnemy[i].flag_VHL & 0x02) >> 1;
		down = (mEnemy[i].flag_VHL & 0x04) >> 2;

		if(((mEnemy[i].yPosition + sizeY) >= GAME_ENEMY_MAX_Y) && (alive == 1))
		{
			flag = 1;
			i = GAME_ENEMY_NUM_ENEMY;
			break;
		}
	}
	
	//set the direction to up
	if (flag == 1)
	{
		for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
		{
			mEnemy[i].flag_VHL &=~ BIT2;		//clear Vertical bit
			mEnemy[i].yPosition -= 1;			//move up
		}
	}
	
	
	//check for direction change down
	flag = 0;
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		//read the flags
		alive = mEnemy[i].flag_VHL & 0x01;
		right = (mEnemy[i].flag_VHL & 0x02) >> 1;
		down = (mEnemy[i].flag_VHL & 0x04) >> 2;

		if((mEnemy[i].yPosition <= GAME_ENEMY_MIN_Y) && (alive == 1))
		{
			flag = 1;
			i = GAME_ENEMY_NUM_ENEMY;
			break;
		}
	}
	
	//set the direction to down
	if (flag == 1)
	{
		for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
		{
			mEnemy[i].flag_VHL |= BIT2;		//set Vertical bit
			mEnemy[i].yPosition += 1;		//move down
		}
	}
}




/////////////////////////////////////////////////
//Game_missileMove()
//
//Update the position of the missiles.  Enemy
//missiles move down and player missiles move up
void Game_missileMove(void)
{
	uint8_t i, j = 0;
	uint8_t mX, mY, bot, top, left, right = 0x00;
	uint8_t numEnemyRemaining = 0x00;
	uint8_t numPlayerRemaining = 0x00;
	
		
	for (i = 0 ; i < GAME_MISSILE_NUM_MISSILE ; i++)
	{
		//player missile - moving up
		if ((mPlayerMissile[i].y > GAME_MISSILE_MIN_Y) && (mPlayerMissile[i].alive == 1))
			mPlayerMissile[i].y-=2;
		else
		{
			//remove it from the alive list
			mPlayerMissile[i].alive = 0;
			mPlayerMissile[i].x = 0;
			mPlayerMissile[i].y = 0;
		}
		
		//check for player missile hit enemy
		if (mPlayerMissile[i].alive == 1)
		{
			//loop over each enemy, did the missile hit it
			for (j = 0 ; j < GAME_ENEMY_NUM_ENEMY ; j++)
			{
				//alive?
				if ((mEnemy[j].flag_VHL & 0x01))
				{
					//get the coordinates of the missile and
					//box that outlines the enemy
					mX = mPlayerMissile[i].x;
					mY = mPlayerMissile[i].y;
					
					bot = mEnemy[j].yPosition + GAME_ENEMY_HEIGHT;
					top = mEnemy[j].yPosition;
					left = mEnemy[j].xPosition + GAME_IMAGE_MARGIN;
					right = mEnemy[j].xPosition + GAME_ENEMY_WIDTH - GAME_IMAGE_MARGIN;
					
                    //tip of the missile in the enemy box?
                    if ((mX >= left) && (mX <= right) && (mY <= bot) && (mY >= top))
                    {
                        //score hit!! - pass enemy index and missile index
                    	//numEnemyRemaining = 0x00;
                    	numEnemyRemaining = Game_scoreEnemyHit(j, i);
                    	
                    	//set the enemy hit flag
                    	mEnemyHitFlag = 1;
                    	
                    	if (numEnemyRemaining == 0)
                    	{
                    		//reset the enemy
                    		Game_levelUp();

                    	}
                    }
				}
			}
		}
				
		//enemy missile - moving down
		if ((mEnemyMissile[i].y < GAME_MISSILE_MAX_Y) && (mEnemyMissile[i].alive == 1))
			mEnemyMissile[i].y+=2;
		else
		{
			mEnemyMissile[i].alive = 0;
			mEnemyMissile[i].x = 0;
			mEnemyMissile[i].y = 0;
		}
		
		//check for enemy missile hitting player
		if (mEnemyMissile[i].alive == 1)
		{
			//get the location of the missile and 
			//compare with the player location.  since the
			//player is outside the framebuffer, it uses x only
			//in the player footprint
			mX = mEnemyMissile[i].x;
			mY = mEnemyMissile[i].y;

			left = mPlayer.xPosition - FRAME_BUFFER_OFFSET_X + GAME_IMAGE_MARGIN;
			right = mPlayer.xPosition + GAME_PLAYER_WIDTH - FRAME_BUFFER_OFFSET_X - GAME_IMAGE_MARGIN;

			top = GAME_MISSILE_MAX_Y - 2;
			bot = GAME_MISSILE_MAX_Y;

            //tip of the missile in the player box?
            if ((mX >= left) && (mX <= right) && (mY <= bot) && (mY >= top))
            {
            	//score player hit
            	numPlayerRemaining = Game_scorePlayerHit(i);
            	
            	//set the player hit flag - polled and cleared in main
            	mPlayerHitFlag = 1;
            	            	
            	//last player??
            	if (numPlayerRemaining == 0)
            	{
            		//set the game over flag, cleared in main
            		mGameOverFlag = 1;
            	}
            }
		}
	}
}




//////////////////////////////////////////
//Draw player on the player page
//updates the contents of the display
void Game_playerDraw(void)
{
	LCD_clearPlayerPage(0x00);
	
	if (mPlayer.numLives > 0)
		LCD_drawImagePage(GAME_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER);
}


//////////////////////////////////////////////
//Draw enemy array into framebuffer.  Does not
//update the contents of the LCD
void Game_enemyDraw(void)
{
	uint8_t i = 0;
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		if ((mEnemy[i].flag_VHL) & 0x01)
		{			
			LCD_drawImageRam(mEnemy[i].xPosition, mEnemy[i].yPosition, BITMAP_ENEMY, 0, 0);
		}
	}
}


//////////////////////////////////////////////////
//Draw missile objects for all missiles
//that are alive.  Applies to be the player
//array of missiles and the enemy array of missiles
//Missiles are a blob of pixels, no image is associated
//with the missile
void Game_missileDraw(void)
{
	uint8_t i = 0;
	
	for (i = 0 ; i < GAME_MISSILE_NUM_MISSILE ; i++)
	{
		//player missile
		if (mPlayerMissile[i].alive == 1)
		{
			LCD_putPixelRam(mPlayerMissile[i].x, mPlayerMissile[i].y, 1, 0);
			LCD_putPixelRam(mPlayerMissile[i].x, mPlayerMissile[i].y - 1, 1, 0);
			
			LCD_putPixelRam(mPlayerMissile[i].x + 1, mPlayerMissile[i].y, 1, 0);
			LCD_putPixelRam(mPlayerMissile[i].x + 1, mPlayerMissile[i].y - 1, 1, 0);
		}
		
		//enemy missile
		if (mEnemyMissile[i].alive == 1)
		{
			LCD_putPixelRam(mEnemyMissile[i].x, mEnemyMissile[i].y, 1, 0);
			LCD_putPixelRam(mEnemyMissile[i].x, mEnemyMissile[i].y + 1, 1, 0);
			
			LCD_putPixelRam(mEnemyMissile[i].x + 1, mEnemyMissile[i].y, 1, 0);
			LCD_putPixelRam(mEnemyMissile[i].x + 1, mEnemyMissile[i].y + 1, 1, 0);
		}
	}
}


//////////////////////////////////////////
//Get first missile from array with alive = 0,
//set alive = 1, and x = player x, and y = player y
void Game_missilePlayerLaunch(void)
{
	uint8_t i = 0;
	uint8_t index = 0;
	uint8_t isAvailable = 0x00;
	
	//get the index of the first missile with alive = 0
	for (i = 0 ; i < GAME_MISSILE_NUM_MISSILE ; i++)
	{
		if (mPlayerMissile[i].alive == 0)
		{
			isAvailable = 1;
			index = i;
			i = GAME_MISSILE_NUM_MISSILE;
			break;
		}		
	}

	if (isAvailable == 1)
	{
		//set the missile
		mPlayerMissile[index].alive = 1;
		mPlayerMissile[index].y = GAME_MISSILE_MAX_Y - 2;
		mPlayerMissile[index].x = mPlayer.xPosition + GAME_MISSILE_OFFSET_X - FRAME_BUFFER_OFFSET_X;		
	}
}


///////////////////////////////////////////
//Launch missile from a random enemy
void Game_missileEnemyLaunch(void)
{
	uint8_t i = 0;
	int index = 0x00;
	uint8_t missileIndex = 0x00;
	uint8_t isAvailable = 0x00;
	
	index = Game_enemyGetRandomEnemy();
	
	if (index >= 0)
	{
		//find the first available missile from enemy
		//missile array and set it to true, x and y
		//to the enemy
		for (i = 0 ; i < GAME_MISSILE_NUM_MISSILE ; i++)
		{
			if (mEnemyMissile[i].alive == 0)
			{
				isAvailable = 1;				//there is one available
				missileIndex = i;
				i = GAME_MISSILE_NUM_MISSILE;
				break;
			}
		}
		
		if (isAvailable == 1)
		{
			//launch
			mEnemyMissile[missileIndex].alive = 1;
			mEnemyMissile[missileIndex].x = mEnemy[index].xPosition + GAME_ENEMY_OFFSET_X;
			mEnemyMissile[missileIndex].y = mEnemy[index].yPosition + GAME_ENEMY_HEIGHT;
		}		
	}
}


///////////////////////////////////////////
//Returns number of enemy with flag alive = 1
uint8_t Game_enemyGetNumEnemy(void)
{
	uint8_t i = 0;
	uint8_t count = 0;
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		if ((mEnemy[i].flag_VHL & 0x01) == 1)
			count++;
	}
	
	return count;
}


//////////////////////////////////////////
//Return the index of a random live enemy
int Game_enemyGetRandomEnemy(void)
{
	uint8_t i = 0;
	uint8_t index = 0;
	uint8_t counter = 0x00;
	uint8_t numEnemy = Game_enemyGetNumEnemy();

	if (numEnemy > 0)
	{
		//get a random index value, zero-based
		//of the enemy to shoot the missile
		index = (uint8_t)(rand() % numEnemy);
		
		//count "index number of values" down the array
		//increment on only alive enemy

		//go to the random index, skipping over dead enemy
		for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
		{
			if (((mEnemy[i].flag_VHL & 0x01)) == 1)
			{
				if (index == counter)
					return counter;
				
				counter++;
			}
		}
	}
	
	return -1;		//no enemy remaining
}



///////////////////////////////////////////////////
//Missile hit enemy.  Return number of enemy
//remaining.  Remove the enemny as alive and
//missile as alive.  Update the player score
uint8_t Game_scoreEnemyHit(uint8_t enemyIndex, uint8_t missileIndex)
{
	uint8_t remaining = 0x0;
	
	//clear the index of the enemy
	mEnemy[enemyIndex].flag_VHL = 0x00;
	mEnemy[enemyIndex].xPosition = 0x00;
	mEnemy[enemyIndex].yPosition = 0x00;
	
	//clear the missile from the player
	mPlayerMissile[missileIndex].alive = 0;
	mPlayerMissile[missileIndex].x = 0x00;
	mPlayerMissile[missileIndex].y = 0x00;
	
	//update the score
	mGameScore += GAME_ENEMY_POINTS;

	remaining = Game_enemyGetNumEnemy();
	
	return remaining;
}


/////////////////////////////////////////////
//Enemy missile hit the player
//returns the number of players remaining
uint8_t Game_scorePlayerHit(uint8_t missileIndex)
{
	mEnemyMissile[missileIndex].alive = 0x00;
	mEnemyMissile[missileIndex].x = 0x00;
	mEnemyMissile[missileIndex].y = 0x00;
			
	if (mPlayer.numLives > 1)
		mPlayer.numLives--;		
	
	else
		mPlayer.numLives = 0;
	
	return mPlayer.numLives;
}


/////////////////////////////////////////
//Cleared the enemy array
//update the game level, reset the enemy arrays
//and missile arrays
void Game_levelUp(void)
{
	mGameLevel++;				//increase the level
	Game_enemyInit();			//reset the enemy
	Game_missileInit();			//reset the missiles
	mGameLevelUpFlag = 1;		//checked, cleared in main
}



///////////////////////////////////////////
uint16_t Game_getGameScore(void)
{
	return mGameScore;
}

///////////////////////////////////////////
uint8_t Game_getGameLevel(void)
{
	return mGameLevel;
}

///////////////////////////////////////////
uint8_t Game_getNumPlayers(void)
{
	return mPlayer.numLives;
}

////////////////////////////////////////////
//Flags - These are typically set object move
//functions and checked in the main game loop
//Flag is used to play sound, an image sequence
uint8_t Game_flagGetPlayerHitFlag(void)
{
	return mPlayerHitFlag;
}

/////////////////////////////////////////////
void Game_flagClearPlayerHitFlag(void)
{
	mPlayerHitFlag = 0;
}

/////////////////////////////////////////////
uint8_t Game_flagGetGameOverFlag(void)
{
	return mGameOverFlag;
}

////////////////////////////////////////////
void Game_flagClearGameOverFlag(void)
{
	mGameOverFlag = 0x00;
}

////////////////////////////////////////////
uint8_t Game_flagGetLevelUpFlag(void)
{
	return mGameLevelUpFlag;
}

///////////////////////////////////////////
void Game_flagClearLevelUpFlag(void)
{
	mGameLevelUpFlag = 0x00;
}

///////////////////////////////////////////
uint8_t Game_flagGetEnemyHitFlag(void)
{
	return mEnemyHitFlag;
}

///////////////////////////////////////////
void Game_flagClearEnemyHitFlag(void)
{
	mEnemyHitFlag = 0x00;
}

/////////////////////////////////////////////
//Flags - Button Flag - Fire button
//Set in button press ISR, and checked in the
//main game loop
void Game_flagSetButtonPress(void)
{
	mButtonFlag = 1;
}

////////////////////////////////////////////
uint8_t Game_flagGetButtonPress(void)
{
	return mButtonFlag;
}


//////////////////////////////////////////////
void Game_flagClearButtonPress(void)
{
	mButtonFlag = 0;
}


//////////////////////////////////////////////
//Draw player ship explosion with delay
//player explodes.  Should remove all missiles
//in the buffer so you can't see them hanging
//
void Game_playExplosionPlayer(void)
{
	DisableInterrupts;
	Game_missileInit();
	LCD_clearFrameBuffer(0x00, 0);
	Game_enemyDraw();
	Game_missileDraw();
	LCD_updateFrameBuffer();
	EnableInterrupts;
	
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP1);
	RTC_delay(20);
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP2);
	RTC_delay(20);
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP3);
	RTC_delay(20);
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP4);
	RTC_delay(20);
}


//////////////////////////////////////////////
//Draw player ship explosion with delay
//player explodes.  Should remove all missiles
//in the buffer so you can't see them hanging
//This version adds sound to the sequence
//Interrupts are disabled for clearing the display
void Game_playExplosionPlayer_withSound(void)
{
	DisableInterrupts;
	Game_missileInit();
	LCD_clearFrameBuffer(0x00, 0);
	Game_enemyDraw();
	Game_missileDraw();
	LCD_updateFrameBuffer();
	EnableInterrupts;
	
	PWM_setFrequency(200);
	GPIO_setRed();
	PWM_Enable();
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP1);
	RTC_delay(20);

	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP2);
	GPIO_clearRed();
	PWM_Disable();
	RTC_delay(20);
	
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP3);
	PWM_setFrequency(300);
	GPIO_setRed();
	PWM_Enable();
	RTC_delay(20);
	
	LCD_drawImagePage(LCD_PLAYER_PAGE, mPlayer.xPosition, BITMAP_PLAYER_EXP4);
	GPIO_clearRed();
	PWM_Disable();
	RTC_delay(20);
	
	PWM_setFrequency(200);
	GPIO_setRed();
	PWM_Enable();
	RTC_delay(20);
	GPIO_clearRed();
	PWM_Disable();	
}


////////////////////////////////////////////
//Game Over
//Draw game over on the screen.  Init the game
//draw a sequence on the boarder, flip from AA to 55
void Game_playGameOver(void)
{
	static uint8_t toggle = 0x00;
	
	DisableInterrupts;
	LCD_clearFrameBuffer(0x00, 0);
	LCD_updateFrameBuffer();	
	EnableInterrupts;
	
	//draw game over
	LCD_drawString(2, 34, "Game");
	LCD_drawString(3, 34, "Over");
	LCD_drawString(5, 31, "Press");	
	LCD_drawString(6, 27, "Button");
	
	if (toggle == 1)
	{
		LCD_clearBackground(0xAA);
		toggle = 0x00;
	}
	else
	{
		LCD_clearBackground(0x55);
		toggle = 0x01;
	}
}







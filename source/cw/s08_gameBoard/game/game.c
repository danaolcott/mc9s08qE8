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
#include "config.h"
#include "game.h"
#include "lcd.h"
#include "bitmap.h"

//Game objects
//Note: Enemy array should be declared as static or 
//indeterminate behavior will result.  ie, declare 6
//enemys and only 5 will get drawn.  Change the spacing
//and only 4 will show up.  Chnage 8bit to 16bit x and y
//coordinates, and also changes the result.  This is only
//for the array objects.  The draw image function works fine
//for any location on the screen.

//static far - puts the array into .bss.
//no static far  - puts the array into common
//no static no far - puts it in common


static PlayerStruct mPlayer = {0x00};

//EnemyStruct mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x84 - common
//EnemyStruct far mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x84 - common
//static EnemyStruct far mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x6C - bss
//static EnemyStruct mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x6C - bss

static EnemyStruct mEnemy[GAME_ENEMY_NUM_ENEMY] = {0x00};		//0x6C - bss



void Game_init(void)
{
	Game_playerInit();
	Game_enemyInit();
	
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


//////////////////////////////////////////
//Draw player on the player page
//updates the contents of the display
void Game_playerDraw(void)
{
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








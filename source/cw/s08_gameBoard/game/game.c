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

//static PlayerStruct mPlayer = {0x00};
static PlayerStruct far mPlayer;

//EnemyStruct mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x84 - common
//EnemyStruct far mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x84 - common
//static EnemyStruct far mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x6C - bss
//static EnemyStruct mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x6C - bss

static EnemyStruct far mEnemy[GAME_ENEMY_NUM_ENEMY];		//0x6C - bss


//flags - ememy movement flag
//bit0 - H - right/left - 0 = right, 1 = left
//bit 1 - V - down/up - 0 = down, 1 = up
//static uint8_t mFlag_enemyDirection_VH = 0x00;



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
	
//	mFlag_enemyDirection_VH = 0x00;			//moving right and down

	

	for (i = 0 ; i < GAME_ENEMY_NUM_ROWS ; i++)
	{
		for (j = 0 ; j < GAME_ENEMY_NUM_COLS ; j++)
		{
			mEnemy[index].alive = 1;
			mEnemy[index].xPosition = (uint8_t)(GAME_ENEMY_X_SPACING * j);
			mEnemy[index].yPosition = (uint8_t)(GAME_ENEMY_Y_SPACING * i);

			index++;
		}
	}
	
}


/*
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
	uint8_t left = mFlag_enemyDirection_VH & 0x01;		//left = 1, right = 0
	uint8_t up = (mFlag_enemyDirection_VH >> 1) & 0x01;	//up = 1, down = 0
	
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		//moving right
		if (!left)
		{
			if (((mEnemy[i].xPosition + sizeX) < GAME_ENEMY_MAX_X) && (mEnemy[i].alive == 1))
				mEnemy[i].xPosition += 2;

		}
		
		//moving left
		else
		{
			if ((mEnemy[i].xPosition > GAME_ENEMY_MIN_X) && (mEnemy[i].alive == 1))
				mEnemy[i].xPosition -= 2;
		}
	}
	
	//direction change - left
	flag = 0x00;	
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		if (((mEnemy[i].xPosition + sizeX) >= GAME_ENEMY_MAX_X) && (mEnemy[i].alive == 1))
		{
			flag = 1;
			i = GAME_ENEMY_NUM_ENEMY;
			break;
		}
	}
	
	if (flag == 1)
		mFlag_enemyDirection_VH |= 0x01;		//switch bit 0 high for left
	
	//direction change - right
	flag = 0x00;	
	for (i = 0 ; i < GAME_ENEMY_NUM_ENEMY ; i++)
	{
		if ((mEnemy[i].xPosition <= GAME_ENEMY_MIN_X) && (mEnemy[i].alive == 1))
		{
			flag = 1;
			i = GAME_ENEMY_NUM_ENEMY;
			break;
		}
	}
	
	if (flag == 1)
	{
		mFlag_enemyDirection_VH &=~ 0x01;		//switch bit 0 low for right

		//all the code that moves the enemy down and up goes here
	}
		
	
}

*/


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
		if (mEnemy[i].alive == 1)
		{			
			LCD_drawImageRam(mEnemy[i].xPosition, mEnemy[i].yPosition, BITMAP_ENEMY, 0, 0);
		}
	}
}








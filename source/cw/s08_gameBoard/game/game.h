/*
 * game.h
 *
 *  Created on: Sep 8, 2019
 *      Author: danao
 * 
 * Configuration file for the game.  Contains instances of 
 * the player, enemy, and missile arrays, all functions for
 * movement, launching missiles, etc.  
 */

#ifndef GAME_H_
#define GAME_H_

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include "config.h"
#include "lcd.h"

#define GAME_PLAYER_PAGE			7
#define GAME_PLAYER_WIDTH			16
#define GAME_PLAYER_MIN_X			FRAME_BUFFER_OFFSET_X
#define GAME_PLAYER_MAX_X			(LCD_WIDTH - FRAME_BUFFER_OFFSET_X - GAME_PLAYER_WIDTH)
#define GAME_PLAYER_DEFAULT_X		40
#define GAME_PLAYER_NUM_LIVES		3

#define GAME_ENEMY_NUM_ENEMY		8
#define GAME_ENEMY_NUM_ROWS			2
#define GAME_ENEMY_NUM_COLS			4
#define GAME_ENEMY_X_SPACING		12
#define GAME_ENEMY_Y_SPACING		10

#define GAME_ENEMY_MIN_X			0
#define GAME_ENEMY_MAX_X			64
#define GAME_ENEMY_MIN_Y			2
#define GAME_ENEMY_MAX_Y			30
#define GAME_ENEMY_OFFSET_X			4
#define GAME_ENEMY_HEIGHT			8
#define GAME_ENEMY_WIDTH			12

#define GAME_MISSILE_NUM_MISSILE	4
#define GAME_MISSILE_MIN_Y			4
#define GAME_MISSILE_MAX_Y			40
#define GAME_MISSILE_SIZE_X			2
#define GAME_MISSILE_SIZE_Y			4
#define GAME_MISSILE_OFFSET_X		8

#define GAME_IMAGE_MARGIN			1

#define GAME_FLAG_PLAYER_HIT		BIT0

/////////////////////////////////////
//Button Type_t
typedef enum
{
	BUTTON_LEFT = 3,
	BUTTON_RIGHT = 2,
	BUTTON_FIRE = 1,
	BUTTON_NONE = 0
}ButtonType_t;

/////////////////////////////////////
//Player Definition
typedef struct{
	uint8_t numLives;
	uint8_t xPosition;
}PlayerStruct;


////////////////////////////////////////
//Enemy Definition
//flag_VHL - bits containing the direction and life
//of the enemy.  V - vertical, high = down
//H = horizontal, high = right, L = life, high = alive
//
typedef struct{
	uint8_t flag_VHL;		//Vertical - down, Horiz - left, Life - alive
	uint8_t xPosition;
	uint8_t yPosition;	
}EnemyStruct;


//////////////////////////////////////////
//Missile Definition
typedef struct
{
	uint8_t alive;
	uint8_t x;
	uint8_t y;	
}MissileStruct;



///////////////////////////////////////////
//Prototypes
void Game_dummyDelay(unsigned int time);

void Game_init(void);
void Game_playerInit(void);
void Game_enemyInit(void);
void Game_missileInit(void);

void Game_playerMoveLeft(void);
void Game_playerMoveRight(void);
void Game_playerMoveDemo(void);


void Game_enemyMove(void);
void Game_missileMove(void);

void Game_playerDraw(void);
void Game_enemyDraw(void);
void Game_missileDraw(void);

void Game_missilePlayerLaunch(void);
void Game_missileEnemyLaunch(void);

uint8_t Game_enemyGetNumEnemy(void);
int Game_enemyGetRandomEnemy(void);

uint8_t Game_scoreEnemyHit(uint8_t enemyIndex, uint8_t missileIndex);
uint8_t Game_scorePlayerHit(uint8_t missileIndex);
void Game_levelUp(void);


//flags
uint8_t Game_flagGetPlayerHitFlag(void);
void Game_flagClearPlayerHitFlag(void);

void Game_flagSetButtonPress(ButtonType_t button);
ButtonType_t Game_flagGetButtonPress(void);
void Game_flagClearButtonPress(ButtonType_t button);


//player explosion
void Game_playExplosionPlayer(void);


///////////////////////////////////

#endif /* GAME_H_ */

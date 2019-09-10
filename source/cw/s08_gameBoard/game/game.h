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


#define GAME_PLAYER_PAGE			7
#define GAME_PLAYER_MIN_X			0
#define GAME_PLAYER_MAX_X			60
#define GAME_PLAYER_DEFAULT_X		40
#define GAME_PLAYER_NUM_LIVES		3

#define GAME_ENEMY_NUM_ENEMY		8
#define GAME_ENEMY_NUM_ROWS			2
#define GAME_ENEMY_NUM_COLS			4
#define GAME_ENEMY_X_SPACING		12
#define GAME_ENEMY_Y_SPACING		10

#define GAME_ENEMY_MIN_X			0
#define GAME_ENEMY_MAX_X			64
#define GAME_ENEMY_MIN_Y			0
#define GAME_ENEMY_MAX_Y			30




/////////////////////////////////////
//Player Definition
typedef struct{
	uint8_t numLives;
	uint8_t xPosition;
}PlayerStruct;


////////////////////////////////////////
//Enemy Definition
typedef struct{
	uint8_t alive;		//alive or not
	uint8_t xPosition;
	uint8_t yPosition;	
}EnemyStruct;





///////////////////////////////////////////
//Prototypes
void Game_init(void);
void Game_playerInit(void);
void Game_enemyInit(void);

//void Game_enemyMove(void);

void Game_playerDraw(void);
void Game_enemyDraw(void);





///////////////////////////////////

#endif /* GAME_H_ */

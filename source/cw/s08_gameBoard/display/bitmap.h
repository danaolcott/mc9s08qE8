/*
 * bitmap.h
 *
 *  Created on: Sep 4, 2019
 *      Author: danao
 *  
 *  Icons and Images Used in the Game
 */

#ifndef BITMAP_H_
#define BITMAP_H_

#include "config.h"

typedef enum Image
{
	BITMAP_PLAYER,
	BITMAP_PLAYER_EXP1,
	BITMAP_PLAYER_EXP2,
	BITMAP_PLAYER_EXP3,
	BITMAP_PLAYER_EXP4,	
	BITMAP_ENEMY
}Image_t;

typedef struct ImageData
{
	uint8_t xSize;
	uint8_t ySize;
	uint8_t *far pImageData;
};

typedef struct ImageData ImageData;

extern const ImageData bmimgPlayerInvBmp;
extern const ImageData bmimgPlayerInvExp1Bmp;
extern const ImageData bmimgPlayerInvExp2Bmp;
extern const ImageData bmimgPlayerInvExp3Bmp;
extern const ImageData bmimgPlayerInvExp4Bmp;


extern const ImageData bmenemy1Bmp;


#endif /* BITMAP_H_ */

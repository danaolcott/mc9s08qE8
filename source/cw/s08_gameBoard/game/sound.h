/*
 * sound.h
 *
 *  Created on: Sep 18, 2019
 *      Author: danao
 * 
 * The purpose of this file is to create a sound scheme
 * that uses PWM to generate a frequency and a timer
 * for a duration.  
 */

#ifndef SOUND_H_
#define SOUND_H_

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"


//////////////////////////////////////////
//SoundData type
//length - number of points in the sound array
//pSoundData - values in khz, 1, 2, 3,... etc
//
typedef struct SoundData
{
	uint8_t length;
	uint8_t *far pSoundData;
};

typedef struct SoundData SoundData;





//sound arrays
extern const SoundData soundBeep;



/////////////////////////////////////////
//Function prototypes
void Sound_init(void);

//Non-blocking methods for playing sound
void Sound_playSound(void);
void Sound_InterruptHandler(void);

//Blocking methods for playing sound
void Sound_playPlayerFire_blocking(void);
void Sound_playEnemyFire_blocking(void);
void Sound_playPlayerExplode_blocking(void);
void Sound_playEnemyExplode_blocking(void);
void Sound_playLevelUp_blocking(void);
void Sound_playGameOver_blocking(void);



#endif /* SOUND_H_ */

/*
 * gpio.h
 *
 *  Created on: Aug 30, 2019
 *      Author: danao
 * 
 *  The purpose of this file is to configure
 *  gpio pins for the leds and user buttons
 * 
 */

#ifndef GPIO_H_
#define GPIO_H_



void GPIO_init(void);
void GPIO_toggleRed(void);
void GPIO_toggleGreen(void);

#endif /* GPIO_H_ */
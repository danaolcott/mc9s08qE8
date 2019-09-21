/*
 * pwm.h
 *
 *  Created on: Aug 17, 2019
 *      Author: danao
 */

#ifndef PWM_H_
#define PWM_H_

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"

#define PWM_MIN_FREQ				100
#define PWM_MAX_FREQ				20000
#define PWM_FREQ_INCREMENT			500
#define PWM_DEFAULT_FREQ			1000

void PWM_init(unsigned long freq);
void PWM_setFrequency(unsigned long freq);

void PWM_setFreq_kHz(uint8_t far freq);


void PWM_Enable(void);
void PWM_Disable(void);
uint8_t PWM_isEnabled(void);

#endif /* PWM_H_ */

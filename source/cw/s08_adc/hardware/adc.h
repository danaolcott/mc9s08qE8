/*
 * adc.h
 *
 *  Created on: May 6, 2019
 *      Author: danao
 * 
 * Controller file for configuring the ADC peripheral
 * on the MC9S08QE8.
 * 
 */

#ifndef ADC_H_
#define ADC_H_

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"


typedef enum
{
	ADC_CHANNEL_8 = 0x08,
	ADC_CHANNEL_9 = 0x09,
	ADC_CHANNEL_TEMP_SENSOR = 0x1A,
	ADC_CHANNEL_VREFL = 0x0A,
	ADC_CHANNEL_VREFH = 0x1D,
	ADC_CHANNEL_VSS = 0x1E
}ADC_Channel_t;

void ADC_init(void);
uint16_t ADC_read(ADC_Channel_t channel);
uint8_t ADC_isValidChannel(ADC_Channel_t channel);



#endif /* ADC_H_ */

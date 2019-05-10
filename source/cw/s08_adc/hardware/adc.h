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

////////////////////////////////////////////////////////
//See Table 17 in the spec sheet.  Values used to compute
//the temp from the internal temp sensor.  Values in 1000's 
//of mV

#define ADC_TEMP25_MV				701200
#define ADC_TEMP_SLOPE_UNDER25		1646		//1.646 - slope of the mV/C curve
#define ADC_TEMP_SLOPE_OVER25		1769		//1.769 - slope of the mV/C curve
#define ADC_VREFH					3260

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
uint16_t ADC_readMv(ADC_Channel_t channel);
int16_t ADC_readTemp(void);
uint8_t ADC_isValidChannel(ADC_Channel_t channel);



#endif /* ADC_H_ */

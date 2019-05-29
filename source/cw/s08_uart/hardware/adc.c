/*////////////////////////////////////////////////////////////
 * adc.c
 *
 *  Created on: May 6, 2019
 *      Author: danao
 *      
 * Controller file for configuring the ADC peripheral
 * on the MC9S08QE8.  For purposes of this project, 
 * configure channels:
 * ADP8 - PTA6 - Pin 22
 * ADP9 - PTA7 - Pin 21
 * 
 * 
 * Channel configuration:
 * ADCH - 5 bit value cooresponding to the channel.  
 * ie, CH8 = 0x000 01000
 * CH9 = 0x000 01001
 * 
 * Temp sensor - AD26, or value 0x000 11010
 * 
 * Can also read the reference voltages, Vrefl, Vss, Vrefh
 * Vrefh = Vdda = 000 11101
 * 
 * Hardware trigger - use the RTC clock to trigger the ADC using the trigger bit
 * 
 * Temperature sensor: Temp = 25 - ((Vtemp - Vtemp25) / m) - See Section 10.1.2.6
 * 
 * /////////////////////////////////////////////////////////
 * Register Definitions:
 * 
 * ADCSC1 - status and control register
 * ADCSC2 - status and control register
 * ADCRH and ADCRL - Data registers - results
 * ADCCFG - configuration register
 * APCTL1, APCTL2, APCTL3 - Pin control registers
 * 
 * 
 *      
 */

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"
#include "adc.h"


/////////////////////////////////////////////////////
//Configure ADC on channels 8 and 9, 12bit conversion
//See Section 10 in the datasheet.
// * ADP8 - PTA6 - Pin 22
// * ADP9 - PTA7 - Pin 21
//
//
void ADC_init(void)
{	
	//ADCSC1 - status and control register 1
	ADCSC1_AIEN = 0x00;		//conversion complete interrupt disabled
	ADCSC1_ADCO = 0x00;		//continuous conversion disabled
	
	//ADC channel bits - set to disable initially - 11111
	ADCSC1_ADCH0 = 0x01;	//channel bit 0
	ADCSC1_ADCH1 = 0x01;	//channel bit 1
	ADCSC1_ADCH2 = 0x01;	//channel bit 2
	ADCSC1_ADCH3 = 0x01;	//channel bit 3
	ADCSC1_ADCH4 = 0x01;	//channel bit 4

	//ADCSC2 - status and control register 2
	ADCSC2_ADTRG = 0;		//software trigger
	ADCSC2_ACFE = 0x00;		//compare function disabled

	//ADCCFG - configuration register
	ADCCFG_ADLPC = 0x00;		//high speed conversion

	//clock divider bits - see Table 10-7
	// 00 = input clock
	// 11 = input clock / 8
	ADCCFG_ADIV0 = 0x00;		//clock divider bit 0
	ADCCFG_ADIV1 = 0x00;		//clock divider bit 1
	
	ADCCFG_ADLSMP = 0x00;		//short sample time

	//mode bits - See Table 10-8 - bit conversion
	//00 = 8 bit, 01 = 12 bit conversion, 10 = 10 bit	
	ADCCFG_MODE0 = 0x01;		//mode bit 0
	ADCCFG_MODE1 = 0x00;		//mode bit 1
	
	//input clock select bits - ADICLK
	//00 - bus clock, 01 - bus clock / 2, 10 - alternate clock, 11 - async clock
	ADCCFG_ADICLK0 = 0x00;
	ADCCFG_ADICLK1 = 0x00;
	
	////////////////////////////////////////////////////////
	//	 * APCTL1, APCTL2, APCTL3 - Pin control registers
	// Disables the IO and enable pin as analog
	//APCTL1 - Pins ADPC0 - ADPC7
	//APCTL2 - Pins ADPC8 - ADPC15
	//APCTL3 - Pins ADPC16 - ADPC23
	
	//enable channels 8 and 9.  Note - Temp sensor is on ch26
	APCTL2_ADPC8 = 1;		//enable channel 8 - PA6 - Pin 22
	APCTL2_ADPC9 = 1;		//enable channel 9 - PA7 - Pin 21
		
}


/////////////////////////////////////////////////////
//Configure the read channel and start the conversion
//poll the conversion complete flag and read results
//in ADCRH and ADCRL
uint16_t ADC_read(ADC_Channel_t channel)
{
	uint16_t low = 0x00;
	uint16_t high = 0x00;
	uint16_t result = 0x00;
	
	//set up the read channel - ADCSC1	
	uint8_t value = ADCSC1 & 0xE0;		//clear the channel bits
	
	if (ADC_isValidChannel(channel))
		value |= ((uint8_t)channel);
	else
		//set to default channel - ground
		value |= ((uint8_t)ADC_CHANNEL_VSS);		

	ADCSC1 = value;					//start the conversion - write to ADCSC1
	while (!(ADCSC1_COCO)){};		//poll the conversion complete

	high = ADCRH;					//read high bits
	low = ADCRL;					//read low bits	
	result = (high << 8) | (low);	//result
	
	return result;
}


////////////////////////////////////////////////////
//Returns the number of millivolts read on a channel
//Read the high reference to compute the mv
//
uint16_t ADC_readMv(ADC_Channel_t channel)
{
	unsigned long raw = 0x00;
	unsigned long mv = 0x00;
	unsigned long vrefh = ADC_VREFH;
	raw = ADC_read(channel);		//get raw reading	
	mv = (raw * vrefh) / 0xFFF;		//compute mv
	
	return ((uint16_t)mv);
}

/////////////////////////////////////////////////////
//Read the chip temp.  
//Read the channel associated with the chip temp
//compute the voltage and compare with T25
//NOTE: scale everything into 1000's of mv

//#define ADC_TEMP25_MV				701200
//#define ADC_TEMP_SLOPE_UNDER25		1646		//1.646 - slope of the mV/C curve
//#define ADC_TEMP_SLOPE_OVER25		1769		//1.769 - slope of the mV/C curve

//result returned in 10th's of a deg, ie, 25.6 deg = 256
int16_t ADC_readTemp(void)
{
	long raw = (long)ADC_readMv(ADC_CHANNEL_TEMP_SENSOR);
	long slope = ADC_TEMP_SLOPE_UNDER25;
	long temp = 0x00;
	long cutoff = ADC_TEMP25_MV;
	
	raw = raw * 1000;
	
	//set the appropriate slope
	if (raw > cutoff)
		slope = ADC_TEMP_SLOPE_OVER25;

	//compute the temp - Eq 10.1 in the datasheet
	//temp = 25 - ((Vtemp - Vtemp25) / m)
	//temp = 25000 - (1000*(Vtemp - Vtemp25) / m)	
	temp = 1000 * (raw - cutoff);
	temp = temp / slope;
	temp = 25000 - temp;
	
	//convert to deg F: F = C * 1.8 + 32		
	temp = temp * 18;
	temp = temp / 10;
	temp = temp + 32000;
	temp = temp / 100;

	return ((int16_t)temp);
}


///////////////////////////////////////////////////
//Checks the channel to see if it's in the
//channel list.  returns 1 if valid, 0 if 
//not valid.
uint8_t ADC_isValidChannel(ADC_Channel_t channel)
{
	switch(channel)
	{
		case ADC_CHANNEL_8:				return 1;		break;
		case ADC_CHANNEL_9:				return 1;		break;
		case ADC_CHANNEL_TEMP_SENSOR:	return 1;		break;
		case ADC_CHANNEL_VREFH:			return 1;		break;
		case ADC_CHANNEL_VREFL:			return 1;		break;
		case ADC_CHANNEL_VSS:			return 1;		break;
		default: 						break;	
	}
	
	return 0;
}





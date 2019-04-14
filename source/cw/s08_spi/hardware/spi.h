/*
 * spi.h
 *
 *  Created on: Apr 12, 2019
 *      Author: danao
 */

#ifndef SPI_H_
#define SPI_H_

#include "derivative.h" /* include peripheral declarations */
#include <stddef.h>
#include "config.h"


void SPI_init(void);

void SPI_select(void);
void SPI_deselect(void);
void SPI_tx(uint8_t data);
uint8_t SPI_rx(void);

void SPI_write(uint8_t data);
uint8_t SPI_read(void);
void SPI_writeArray(uint8_t* data, uint16_t length);
void SPI_readArray(uint8_t* data, uint16_t length);

#endif /* SPI_H_ */

/*
 * lcd.h
 *
 *  Created on: Aug 12, 2019
 *      Author: danao
 *  
 *  Header File for the EA Display DOGS102N-6 LCD
 *  102x64 Display with SPI Interface.
 *  
 *  Pinout:
 *  PB0 - Reset - Pin 27
 *  PB1 - CMD/Data - Pin 26
 *  PB2 - SCK - Pin 25
 *  PB3 - MOSI - Pin 24
 *  PB5 - Chip Select
 *  
 *  Screen Layout - 
 *  RAM is limited to 512 bytes.
 *  Framebuffer that is 64 x 48, offset 16 pixels to the 
 *  left and 8 pixels down.  This gives padding around the 
 *  edges of the main display.  Outside area is used for
 *  drawing icons, score, etc.  
 *  
 *  
 */

#ifndef LCD_H_
#define LCD_H_

#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include <stddef.h>
#include "config.h"
#include "bitmap.h"


//defines
#define LCD_RST_PIN		BIT6		//Port C
#define LCD_CMD_PIN		BIT7		//Port C
#define LCD_CS_PIN		BIT5		//Port B


#define LCD_HEIGHT		64
#define LCD_WIDTH		102
#define LCD_NUM_PAGES	8


//////////////////////////////////////////////
//Note:  Frame buffer is not the full size of the
//LCD due to memory constraints.
#define FRAME_BUFFER_SIZE		320
#define FRAME_BUFFER_WIDTH		64
#define FRAME_BUFFER_HEIGHT		40
#define FRAME_BUFFER_OFFSET_X	19
#define FRAME_BUFFER_OFFSET_Y	19
#define FRAME_BUFFER_START_PAGE	2
#define FRAME_BUFFER_STOP_PAGE	6
#define FRAME_BUFFER_NUM_PAGES	5

#define LCD_PLAYER_PAGE				7
#define LCD_SCORE_PAGE				0


//prototypes
void LCD_dummyDelay(unsigned long delay);
void LCD_reset(void);
void LCD_writeCommand(uint8_t cmd);
void LCD_writeData(uint8_t data);
void LCD_writeDataBurst(uint8_t *far data, uint16_t length);

void LCD_init(void);

void LCD_setPage(uint8_t page);
void LCD_setColumn(uint8_t column);

void LCD_clear(uint8_t value);
void LCD_clearPage(uint8_t page, uint8_t value);
void LCD_clearFrameBuffer(uint8_t value, uint8_t update);
void LCD_clearBackground(uint8_t value);
void LCD_updateFrameBuffer(void);

void LCD_drawString(uint8_t row, uint8_t col, char *far myString);
void LCD_drawStringLength(uint8_t row, uint8_t col, char *far mystring, uint8_t length);

uint8_t LCD_decimalToBuffer(unsigned int val, char far* buffer, uint8_t size);

void LCD_drawImagePage(uint8_t x, uint8_t y, Image_t image);

//functions that manipulate the framebuffer
void LCD_putPixelRam(uint16_t x, uint16_t y, uint8_t color, uint8_t update);
void LCD_drawImageRam(uint16_t xPosition, uint16_t yPosition, Image_t image, uint8_t trans, uint8_t update);
void LCD_drawEnemyBitmap(uint16_t xPosition, uint16_t yPosition);

#endif /* LCD_H_ */




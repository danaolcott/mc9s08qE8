/*
 * lcd.c
 *
 *  Created on: Aug 12, 2019
 *      Author: danao
 *  LCD uses a SPI interface with idle clock high
 *  and data clocked in on the trailing edge (rising)
 */



#include <hidef.h> /* for EnableInterrupts macro */
#include "derivative.h" /* include peripheral declarations */
#include "mc9s08qe8.h"
#include <stddef.h>

#include "config.h"
#include "lcd.h"
#include "spi.h"
#include "font_table.h"
#include "gpio.h"


#define abs(a)  ((a)<0?-(a):a)

/////////////////////////////////////////////////////
//Framebuffer allocated to memory location 0x100
volatile uint8_t frameBuffer[FRAME_BUFFER_SIZE] @ 0x100u;


////////////////////////////////////////////////
//Waste CPU cycles
void LCD_dummyDelay(unsigned long delay)
{
	volatile unsigned long temp = delay;
	while (temp > 0)
	{
		temp--;		
	}
}


/////////////////////////////////////////////////
//Toggle the reset pin on portb
void LCD_reset(void)
{
	PTCD |= LCD_RST_PIN;		//high
	PTCD &=~ LCD_RST_PIN;		//low
	LCD_dummyDelay(5000);		//about 100ms
	PTCD |= LCD_RST_PIN;		//high
}


/////////////////////////////////////////////////
//Send one byte to the LCD as a command
//All commands are single byte command
void LCD_writeCommand(uint8_t cmd)
{
	PTCD &=~ LCD_CMD_PIN;		//CMD = low
	SPI_write(cmd);
}

//////////////////////////////////////////////////
//Write single byte as data
void LCD_writeData(uint8_t data)
{
	PTCD |= LCD_CMD_PIN;		//data = high	
	SPI_write(data);	
}

/////////////////////////////////////////////////
//Write data burst
void LCD_writeDataBurst(uint8_t *far data, uint16_t length)
{
	PTCD |= LCD_CMD_PIN;		//data = high	
	SPI_writeArray(data, length);
}



////////////////////////////////////////////
//LCD_init(void)
//Configure the control lines as output
//write intial setup to the lCD
//Assumes the SPI is properly configured prior
//to calling this function
void LCD_init(void)
{
	uint16_t i = 0x00;
	for (i = 0 ; i < FRAME_BUFFER_SIZE ; i++)
		frameBuffer[i] = 0x00;
		
	//Control Lines - Port C
	PTCDD |= LCD_RST_PIN;		//Reset
	PTCDD |= LCD_CMD_PIN;		//Command / Data
	
	PTCD |= LCD_RST_PIN;		//Reset
	PTCD &=~ LCD_CMD_PIN;		//CMD = low
	
	LCD_reset();
	
	//Write initial register settings.
	//See datasheet
	LCD_writeCommand(0x40);		//Display start line 0
	LCD_writeCommand(0xA1);		//SEG reverse
	LCD_writeCommand(0xC0);		//Normal COM0-COM63
	LCD_writeCommand(0xA4);		//Set all pixels to ON
	LCD_writeCommand(0xA6);		//Display inverse off
	LCD_writeCommand(0xA2);		//Set bias 1/9 (Duty 1/65)
	LCD_writeCommand(0x2F);		//Booster, Regulator and Follower on
	LCD_writeCommand(0x27);		//Set Contrast
	LCD_writeCommand(0x81);		//Set Contrast
	LCD_writeCommand(0x10);		//Set Contrast
	
	LCD_writeCommand(0xFA);		//Set temperature compensation
	LCD_writeCommand(0x90);		//Set temperature compensation
	
	LCD_writeCommand(0xAF);		//Display on

	LCD_setPage(0);
	LCD_setColumn(0);
}


//////////////////////////////////////
//Set display enable command
void LCD_on(void)
{
	LCD_writeCommand(0xAF);
}

/////////////////////////////////////////
//Set display disable command
void LCD_off(void)
{
	LCD_writeCommand(0xAE);	
}

/////////////////////////////////////////////
//Set Page - 0 to 7, top to bottom.
//Base address = 0xB0, page address is
//the lower 4 bits
void LCD_setPage(uint8_t page)
{
	uint8_t result = 0xB0 | (page & 0x0F);
	LCD_writeCommand(result);
}


////////////////////////////////////////////
//Set Column - x offset
void LCD_setColumn(uint8_t column)
{
	uint8_t low = 0x00 | (column & 0x0F);		//lower 4 bits
	uint8_t high = 0x10 | (column >> 4);		//upper 4 bits
	
	LCD_writeCommand(low);						//set the low bits
	LCD_writeCommand(high);						//set the high bits	
}


///////////////////////////////////////////
//Base address = 0x80 and contrast setting
//is lower 7 bits - 0 to 63
void LCD_setContrast(uint8_t contrast)
{
	uint8_t value = 0x80;
	value |= (contrast & 0x7F);	
	LCD_writeCommand(value);
}




/////////////////////////////////////////////
//Clear the LCD with a value
//This area includes the framebuffer and borders
//also clears the framebuffer memory
//Clears the score page and player page as well.
void LCD_clear(uint8_t value)
{
	uint8_t i, j;
	uint16_t index = 0x00;
	
	for (index = 0x00 ; index < FRAME_BUFFER_SIZE ; index++)
		frameBuffer[index] = value;

	LCD_setColumn(0);
	LCD_setPage(0);
	
	for (i = 0 ; i < LCD_NUM_PAGES ; i++)
	{
		LCD_setPage(i);			//increment the page
		LCD_setColumn(0);		//reset the x
		
		for (j = 0 ; j < LCD_WIDTH ; j++)
			LCD_writeData(value);
	}	
}




//////////////////////////////////////////////
//Clears the framebuffer memory and updates
//the contents of the display within the 
//framebuffer region only
//if update == 1, update the frame buffer with
//contents of the display
void LCD_clearFrameBuffer(uint8_t value, uint8_t update)
{
	uint8_t i, j;
	uint16_t index = 0x00;
	
	for (index = 0 ; index < FRAME_BUFFER_SIZE ; index++)
		frameBuffer[index] = value;
	
	//update the contents of the display
	if (update == 1)
	{
		index = 0x00;
		
		LCD_setColumn(FRAME_BUFFER_OFFSET_X);
		LCD_setPage(FRAME_BUFFER_START_PAGE);
		
		for (i = FRAME_BUFFER_START_PAGE ; i < FRAME_BUFFER_STOP_PAGE + 1 ; i++)
		{
			LCD_setColumn(FRAME_BUFFER_OFFSET_X);	//reset the x
			LCD_setPage(i);							//increment the page
			
			for (j = 0 ; j < FRAME_BUFFER_WIDTH ; j++)
				LCD_writeData(frameBuffer[index++]);
		}
	}
}


//////////////////////////////////////////////
//Clear the page that is used by the player
//over the full width of the LCD
void LCD_clearPlayerPage(uint8_t value)
{
	uint8_t i = 0x00;
	LCD_setColumn(0);
	LCD_setPage(LCD_PLAYER_PAGE);
	
	for (i = 0 ; i < LCD_WIDTH ; i++)
			LCD_writeData(value);
}

///////////////////////////////////////////////
//Clear the page that is used to show the score
//over the full width of the LCD
void LCD_clearScorePage(uint8_t value)
{
	uint8_t i = 0x00;
	LCD_setColumn(0);
	LCD_setPage(LCD_SCORE_PAGE);

	for (i = 0 ; i < LCD_WIDTH ; i++)
			LCD_writeData(value);
}

//////////////////////////////////////////////
//Clears everything except the framebuffer 
//area, the score page (full width) and player
//page (full width).  Basically, the left and right
//margins over the height of the frame buffer
void LCD_clearBackground(uint8_t value)
{
	uint8_t i, j;			

	for (i = FRAME_BUFFER_START_PAGE ; i < FRAME_BUFFER_STOP_PAGE + 1 ; i++)
	{
		//left margin
		LCD_setColumn(0);
		LCD_setPage(i);
		
		for (j = 0 ; j < FRAME_BUFFER_OFFSET_X ; j++)
			LCD_writeData(value);
		
		//right margin
		LCD_setColumn(LCD_WIDTH - FRAME_BUFFER_OFFSET_X);
		LCD_setPage(i);

		for (j = 0 ; j < FRAME_BUFFER_OFFSET_X ; j++)
			LCD_writeData(value);
	}
}


//////////////////////////////////////////////
//Update the display with the contents of the
//framebuffer.  The buffer is displayed at 
//FRAME_BUFFER_OFFSET_X and FRAME_BUFFER_START_PAGE
//over a height of FRAME_BUFFER_NUM_PAGES
//
void LCD_updateFrameBuffer(void)
{
	uint8_t i, j;
	uint16_t element = 0;
	
	volatile uint8_t *far ptr = frameBuffer;
		
	for (i = FRAME_BUFFER_START_PAGE ; i < FRAME_BUFFER_STOP_PAGE + 1 ; i++)
	{
		LCD_setColumn(FRAME_BUFFER_OFFSET_X);
		LCD_setPage(i);
		LCD_writeDataBurst((uint8_t *far)ptr, FRAME_BUFFER_WIDTH);		
		ptr += FRAME_BUFFER_WIDTH;
	}	
}


////////////////////////////////////////////
//Draws one character at row and col from
//the 8x8 font table
void LCD_drawChar(uint8_t row, uint8_t col, uint8_t letter)
{
	unsigned int line;
	unsigned int value0;
	uint8_t width = 8;
	unsigned int i = 0;
	
	if ((letter >= 32) && (letter < 127))
	{
		//set the x and y start positions
		LCD_setPage(row);
		LCD_setColumn(col);
		
		if ((col + width) < LCD_WIDTH)
		{
			line = letter - 27;		//ie, for char 32 " ", it's on line 5
			value0 = (line-1) << 3;
			
			//loop through the width
			for (i = 0 ; i < width ; i++)
			{
				LCD_writeData(font_table[value0 + i]);
			}
		}		
	}
}



////////////////////////////////////////////////////
//LCD_drawString.
//Print string of characters to the LCD.
//Note: the far keyword is required for this to 
//work since the project is using the tiny memory model.
//If we were using the other memory model, I think you dont 
//need it because the variable would be placed into far region
//of memory by default.  Leaving the keyword out crashes the program
//and using near keyword crashes the program.
//When calling this function, there is no need to cast the input
//parameter as char *far.
void LCD_drawString(uint8_t row, uint8_t col, char *far myString)
{
	uint8_t count = 0;
	uint8_t position = col;

	unsigned int line;
	unsigned int value0;
	uint8_t width = 8;
	unsigned int i = 0;
	
	//set the x and y start positions
	LCD_setPage(row);
	
	while ((myString[count] != 0x00) && (position < LCD_WIDTH))
	{
		LCD_setColumn(position);

		if ((position + width) < LCD_WIDTH)
		{
			line = myString[count] - 27;		//ie, for char 32 " ", it's on line 5
			value0 = (line-1) << 3;

			for (i = 0 ; i < width ; i++)
			{
				LCD_writeData(font_table[value0 + i]);
			}
		}

		position += width;
		count++;
	}
}



//////////////////////////////////////////////////
//Draws a string of characters at starting
//row and col.
void LCD_drawStringLength(uint8_t row, uint8_t col, char *far mystring, uint8_t length)
{
	unsigned int i, j = 0;
	unsigned char position = col;
	uint8_t width = 8;
	unsigned int value0 = 0;
	unsigned int line = 0;

	//set the x and y start positions
	LCD_setPage(row);

	for (i = 0 ; i < length ; i++)
	{
		LCD_setColumn(position);

		if ((position + width) < LCD_WIDTH)
		{
			line = mystring[i] - 27;		//ie, for char 32 " ", it's on line 5
			value0 = (line-1) << 3;

			for (j = 0 ; j < width ; j++)
			{
				LCD_writeData(font_table[value0 + j]);
			}
		}

		position += width;
	}
}



//////////////////////////////////////////
//Print val into buffer of max size size.
//returns the number of characters in the
//the buffer.  Assumes the buffer
//is clear prior to writing to it.
uint8_t LCD_decimalToBuffer(unsigned int val, char far* buffer, uint8_t size)
{
    uint8_t i = 0;
    char digit;
    uint8_t num = 0;
    char t;
    
    //test for a zero value
    if (val == 0)
    {
    	buffer[0] = '0';
    	buffer[1] = 0x00;
    	return 1;
    }

    while (val > 0)
    {
        digit = (char)(val % 10);
        buffer[num] = (0x30 + digit) & 0x7F;
        num++;
        val = val / 10;
    }

    //reverse in place
    for (i = 0 ; i < num / 2 ; i++)
    {
        t = buffer[i];
        buffer[i] = buffer[num - i - 1];
        buffer[num - i - 1] = t;
    }

    buffer[num] = 0x00;     //null terminated

    return num;
}



///////////////////////////////////////////////////////////////
//Draws image onto LCD directly.  Images are assumed to be page
//aligned (width of a multiple of a page) and 1 bit per pixel 
//stored vertically with LSB on top.
//
//Note: The function that draws image into framebuffer assumes
//that the image is stored horizontally, 1 bit per pixel, MSB
//to LSB
void LCD_drawImagePage(uint8_t page, uint8_t offset, Image_t image)
{
	uint8_t element = 0;
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t width, numPages = 0;
	
	//set the image pointer
	const ImageData *far ptr = &bmimgPlayerInvBmp;
	
	switch(image)
	{
		case BITMAP_PLAYER: 		ptr = &bmimgPlayerInvBmp;		break;
		case BITMAP_PLAYER_EXP1: 	ptr = &bmimgPlayerInvExp1Bmp;	break;
		case BITMAP_PLAYER_EXP2: 	ptr = &bmimgPlayerInvExp2Bmp;	break;
		case BITMAP_PLAYER_EXP3: 	ptr = &bmimgPlayerInvExp3Bmp;	break;
		case BITMAP_PLAYER_EXP4: 	ptr = &bmimgPlayerInvExp4Bmp;	break;
		case BITMAP_ENEMY:			ptr = &bmenemy1Bmp;				break;
		case BITMAP_PLAYER_ICON3:	ptr = &bmimgPlayerIcon_3;		break;
		case BITMAP_PLAYER_ICON2:	ptr = &bmimgPlayerIcon_2;		break;
		case BITMAP_PLAYER_ICON1:	ptr = &bmimgPlayerIcon_1;		break;
		default:					ptr = &bmimgPlayerInvBmp;		break;
		
	}
	
	width = ptr->xSize;
	numPages = (ptr->ySize) / 8;
	
	//set the initial position
	LCD_setPage(page);
	LCD_setColumn(offset);
	
	for (i = 0 ; i < numPages ; i++)
	{
		LCD_setColumn(offset);				//reset
		LCD_setPage(page + i);				//increment
		
		for (j = 0 ; j < width ; j++)
		{
			//write the data
			LCD_writeData(ptr->pImageData[element]);
			element++;
		}
	}	
}

/////////////////////////////////////////////////////////
//LCD_putPixelRam
//Modifies a single bit in the framebuffer and updates
//the display when update = 1
//Note:
//Framebuffer width is not the same as LCD_WIDTH
//all x and y aligned to the edge of the frame buffer
//
void LCD_putPixelRam(uint16_t x, uint16_t y, uint8_t color, uint8_t update)
{
	uint16_t element = 0x00;    //frame buffer element
	uint8_t elementValue = 0x00;    
	uint8_t bitShift = 0x00;

    //test for valid input
	if ((x > (FRAME_BUFFER_WIDTH - 1)) || (y > (FRAME_BUFFER_HEIGHT - 1)))
		return;
	
	//element the frame buffer to read / write
	element = ((y >> 3) * FRAME_BUFFER_WIDTH) + x;
	
	//offset - MSB on bottom
	if (y < 8)
		bitShift = (uint8_t)y;
	else if ((y % 8) == 0)
		bitShift = 0;
	else
		bitShift = y % 8;
	
	//read
	elementValue = frameBuffer[element];
	
	//modify
	if (color == 1)
		elementValue |= (1 << bitShift);        //add 1
	else    
		elementValue &=~ (1 << bitShift);       //clear 1
	
    //write
    frameBuffer[element] = elementValue;
	
	//update
	if (update > 0)
	{
		//update the display - offset by position
		//of the framebuffer
		LCD_setColumn(x + FRAME_BUFFER_OFFSET_X);
		LCD_setPage((y >> 3) + FRAME_BUFFER_START_PAGE);
		LCD_writeData(elementValue);
	}	
}



///////////////////////////////////////////////////////////////
void LCD_drawLine(int x0, int y0, int x1, int y1, uint8_t color)
{
	int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
	int dy = abs(y1-y0), sy = y0<y1 ? 1 : -1;
	int err = (dx>dy ? dx : -dy)/2, e2;
	
	for(;;)
	{
		//update the display
		LCD_putPixelRam(x0,y0, color, 1);
		if (x0==x1 && y0==y1) break;
		e2 = err;
		if (e2 >-dx) { err -= dy; x0 += sx; }
		if (e2 < dy) { err += dx; y0 += sy; }
	}   
}





////////////////////////////////////////////////////////////////
//Draw Image into x and y coordinates into the frambuffer
//Since the framebuffer is offset on the LCD, reference the 
//x and y to the edge of the buffer, not the LCD.
//Input images are assumed to be stored as arrays that
//are horizontally aligned, 1 bit per pixel, 8 pixels per
//byte, MSB to LSB.
//
//trans - bit value to ignore.  ie, if trans = 0, ignore and
//skip over 0 pixels (ie, does not clear the pixel), if trans = 1,
//ignore and skip over 1 pixels (ie, don't draw on pixels).

//if update = 1, the LCD is updated with the contents of the
//framebuffer.
//
//Note:  This is opposite to drawing images directly into
//the LCD.  The framebuffer stores the data vertically
//to allow for faster writes.  The conversion is done
//in the put pixel function
void LCD_drawImageRam(uint16_t xPosition, uint16_t yPosition, Image_t image, uint8_t trans, uint8_t update)
{
	uint8_t bitValue = 0;
	uint8_t p = 0;
	uint16_t i = 0;
	uint16_t j = 0;
	uint16_t counter = 0x00;
	uint8_t data = 0x00;
	uint8_t sizeX = 0x00;
	uint8_t sizeY = 0x00;    
	uint16_t x = xPosition;
	uint16_t y = yPosition;
	 
	//set the pointer
	uint8_t *far ptr = bmenemy1Bmp.pImageData;
	sizeX = bmenemy1Bmp.xSize;
	sizeY = bmenemy1Bmp.ySize;
	
	//set the image data pointer
    switch(image)
    {
		case BITMAP_ENEMY:
		{
			ptr = bmenemy1Bmp.pImageData;
			sizeX = bmenemy1Bmp.xSize;
			sizeY = bmenemy1Bmp.ySize;
			break;
		}
		
		default:
		{
			ptr = bmenemy1Bmp.pImageData;
			sizeX = bmenemy1Bmp.xSize;
			sizeY = bmenemy1Bmp.ySize;
		}
    }
         
    for (i = 0 ; i < sizeY ; i++)
    {
		x = xPosition;        //reset the x position
		
		//1bpp - 8 pixels per element
		for (j = 0 ; j < (sizeX / 8) ; j++)
		{   
			data = ptr[counter];
			
			//work counter 8 times
			p = 8;                  //reset p
			while (p > 0)
			{                
				bitValue = (data >> (p-1)) & 0x01;
				
				if (bitValue != trans)
				{
					if (!bitValue)
						LCD_putPixelRam(x, y, 0, update);
					else
						LCD_putPixelRam(x, y, 1, update);
				}
				
				x++;            //increment the x
				p--;
			}
			
			counter++;
		}
		
		y++;        //increment the row
	}
}






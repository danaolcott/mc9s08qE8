/*
Register definitions for the NXP 8bit MC9S08QE8 processor
*/

#ifndef __REGISTER_H
#define __REGISTER_H


/////////////////////////////////////////////
#define BIT0        (1u << 0)
#define BIT1        (1u << 1)
#define BIT2        (1u << 2)
#define BIT3        (1u << 3)
#define BIT4        (1u << 4)
#define BIT5        (1u << 5)
#define BIT6        (1u << 6)
#define BIT7        (1u << 7)



//////////////////////////////////////////////
//GPIO
__xdata __at 0x00 volatile unsigned char PTAD;
__xdata __at 0x01 volatile unsigned char PTADD;


///////////////////////////////////////////////
//System
__xdata __at 0x1F volatile unsigned char CONFIG;




#endif



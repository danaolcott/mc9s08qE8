/*
4/7/19
Blinky Example Project for the MC9S08QE8 Processor

Compile command:

Code Location: 0xE000
Stack Location: 0x00FF      - default stack pointer??

Example:
sdcc -mhc08 --stack-loc 0x00FF --code-loc 0xE000 blink.c


*/



#include <stdint.h>
#include <stddef.h>
#include "register.h"


void delay(uint16_t value);
void GPIO_init(void);


int main()
{
    CONFIG = 0x01;          //initial config - this can only be written to one temp

    GPIO_init();

    while (1)
    {
        PTAD ^= BIT0;
        PTAD ^= BIT1;
        PTAD ^= BIT2;
        PTAD ^= BIT3;

        delay(10000);
    }
}


void delay(uint16_t value)
{
    volatile unsigned int temp = value;

    while (temp > 0)
        temp--;
}


////////////////////////////////////
//Configure PTA Bits 0-3 as output
void GPIO_init(void)
{
    //set initially low
    PTAD &=~ BIT0;
    PTAD &=~ BIT1;
    PTAD &=~ BIT2;
    PTAD &=~ BIT3;

    PTADD |= BIT0;
    PTADD |= BIT1;
    PTADD |= BIT2;
    PTADD |= BIT3;

}





/*
4/7/19
Blinky Example Project for the MC9S08QE8 Processor

Compile command:

Code Location: 0xE000
Stack Location: 0x00FF      - default stack pointer??

Example:
sdcc -mhc08 --stack-loc 0x00FF --code-loc 0xE000 blink.c


*/


//data at 0x00 volatile char PTAD;
//data at 0x01 volatile char PTADD;

#include <stdint.h>
#include <stddef.h>

#define BIT0        (1u << 0)
#define BIT1        (1u << 1)
#define BIT2        (1u << 2)
#define BIT3        (1u << 3)
#define BIT4        (1u << 4)
#define BIT5        (1u << 5)
#define BIT6        (1u << 6)
#define BIT7        (1u << 7)


#define PTAD        (*((volatile uint8_t*)(0x00)))
#define PTADD       (*((volatile uint8_t*)(0x01)))

//Need something with the watchdog timer



//data 0x00 volatile char PTAD;
//data 0x01 volatile char PTADD;
//data at 0x1F volatile char CONFIG;


void delay(uint16_t value);
void GPIO_init(void);


int main()
{
//    CONFIG = 0x01;          //initial config - this can only be written to one temp

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





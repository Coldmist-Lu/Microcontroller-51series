/* ÊµÑé1-7-2 AT24C02 */

#include "reg52.h"
#include "iic.h"
#include "intrins.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;


BYTE code T_display[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

BYTE code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //Î»Âë

BYTE table[2];

//-----------------------------------------------


void Delay10ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 108;
	j = 145;
	do
	{
		while (--j);
	} while (--i);
}


/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define SFR */
sbit TEST_LED = P1^0;               //work LED, flash once per second

/* define variables */
WORD count;                         //1000 times counter

//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static BYTE i;
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 10;
				P2 = 0xe0; P0 = ~T_display[ table[i] ]; P2 = 0;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0;
				i++;
				if(i == 2) i = 0;
    }
}

//-----------------------------------------------

/* main program */
sbit buzzer = P0^6;
sbit relay = P0^4;

void main()
{	
		unsigned char open_num;
	
		P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
		
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter

		open_num = read_AT24C02(0x55);
		
		write_AT24C02(0x55, ++open_num);
	
		table[0] = open_num / 10;
		table[1] = open_num % 10;
		
    while (1) {
			Delay10ms();
		}
}

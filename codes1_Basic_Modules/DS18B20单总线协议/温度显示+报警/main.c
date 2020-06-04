/* 实验1-5-2 温度显示+报警电路  2019.3.3 */

#include "onewire.h"
#include "intrins.h"
#include "reg52.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;

//-----------------------------------------------

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define variables */
WORD count;                         //1000 times counter

//-----------------------------------------------

unsigned char code T_display[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1
unsigned char code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码

BYTE table[3];

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static BYTE i;
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 1;               //reset counter
				if(i == 1) {
						P2 = 0xe0; P0 = ~(T_display[ table[i] ] + 0x80); P2 = 0x00;
				}	else {
						P2 = 0xe0; P0 = ~T_display[ table[i] ]; P2 = 0x00;
				}
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0x00;
				i++;
				if(i == 3) i = 0;
        //blank
    }
}

void Delay10ms()		//@11.0592MHz
{
	unsigned char i, j;

	i = 18;
	j = 235;
	do
	{
		while (--j);
	} while (--i);
}

void timer0_init() {
		TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter
}

//-----------------------------------------------

/* main program */

sbit buzzer = P0 ^ 6;
sbit relay = P0 ^ 4;

void main()
{
		float tpt;
    P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00; 
		P2 = 0x80; P0 = 0xff; P2 = 0x00;
		timer0_init();
    while (1) {
			EA = 0;
			tpt = read_temperature_f();
			EA = 1;
			table[0] = (BYTE)tpt / 10;
			table[1] = (BYTE)tpt % 10;
			table[2] = (BYTE)(tpt * 10) % 10;
			
			if(tpt > 14) {
				P2 = 0xa0; buzzer = 1; relay = 0; P2 = 0x00;
				P2 = 0x80; P0 = ~0x01; P2 = 0x00;
			}
			else if(tpt < 12) {
				P2 = 0xa0; buzzer = 0; relay = 1; P2 = 0x00;
				P2 = 0x80; P0 = ~0x80; P2 = 0x00;
			} else {
				P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
				P2 = 0x80; P0 = ~0x10; P2 = 0x00;
			}
			Delay10ms();
		}			//loop
}



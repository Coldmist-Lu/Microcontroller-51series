#include "intrins.h"
#include "ds1302.h"
#include "reg52.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;


BYTE code T_display[]={                       //±ê×¼×Ö¿â
//   0    1    2    3    4    5    6    7    8    9    
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F, 0x00};
BYTE code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //Î»Âë

BYTE shi, fen, miao;

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




//-----------------------------------------------

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define variables */
WORD count;                         //1000 times counter

BYTE table[10];
//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static BYTE i;
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 1;               //reset counter
        P2 = 0xe0; P0 = ~T_display[ table[i] ]; P2 = 0;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0;
				i++;
				if(i == 8) i = 0;
    }
}

//-----------------------------------------------

sbit buzzer = P0 ^6;
sbit relay = P0 ^ 4;

/* main program */
void main()
{
		P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter

		table[2] = 10;
		table[5] = 10;
	
		set_sfm(23, 59, 55);
	
    while (1) {
			EA = 0;
			shi = Ds1302_Single_Byte_Read(0x85);
			fen = Ds1302_Single_Byte_Read(0x83);
			miao = Ds1302_Single_Byte_Read(0x81);
			EA = 1;
			table[0] = shi / 16;
			table[1] = shi % 16;
			table[3] = fen / 16;
			table[4] = fen % 16;
			table[6] = miao / 16;
			table[7] = miao % 16;
			Delay10ms();
			
		}
}


/* ÊµÑé1-8 ³¬Éù²¨²â¾àÄ£¿é */
#include "reg52.h"
#include "intrins.h"


typedef unsigned char BYTE;
typedef unsigned int WORD;

BYTE code T_display[]={ 0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F };
BYTE code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //Î»Âë

BYTE Sonic_Table[3];
BYTE Sonic_Count;
bit Sonic_Flag;

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
		if (Sonic_Count-- == 0) {
				Sonic_Count = 200;
				Sonic_Flag = 1;
		}
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 1;               //reset counter
        P2 = 0xe0; P0 = ~Sonic_Table[i]; P2 = 0;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0;
				i++;
				if(i == 3) i = 0;
    }
}

//-----------------------------------------------

/* main program */
sbit buzzer = P0 ^6;
sbit relay = P0 ^ 4;
sbit TX = P1^0;
sbit RX = P1^1;

#define somenop {_nop_(); _nop_();_nop_();_nop_();_nop_();}

void send_wave(void) {
		unsigned char i = 8;
		do {
				TX = 1;
				somenop;somenop;somenop;somenop;somenop;
				TX = 0;
				somenop;somenop;somenop;somenop;somenop;
		} while(i--);
}

void main()
{
		unsigned int distance, t;
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter

    while (1) {
			if(Sonic_Flag == 1) {
				Sonic_Flag = 0;
				send_wave();
				TR1 = 1;
				while((RX == 1) && (TF1 == 0));
				TR1 = 0;
				
				if(TF1 == 1) {
					TF1 = 0;
					distance = 999;
				} else {
					t = TH1;
					t <<= 8;
					t |= TL1;
					distance = (unsigned int)(t * 0.017);
				}
				TH1 = 0;
				TL1 = 0;
					
				Sonic_Table[0] = T_display[distance / 100] | 0x80;
				Sonic_Table[1] = T_display[distance / 10 % 10];
				Sonic_Table[2] = T_display[distance % 10];
			}
		}
}


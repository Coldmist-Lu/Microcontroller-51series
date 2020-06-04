/* 实验1-4-3 实现数码管显示对应矩阵键盘 */
/* by Coldmist.Lu 2019.2.21 */
#include "reg52.h"

#define KEYPORT P3

typedef unsigned char u8;

u8 Trg, Cont;

void Read_KBD(void) {
	u8 key1, key2, keyval;
	
	KEYPORT = 0xf0;
	key1 = KEYPORT & 0xf0;
	KEYPORT = 0x0f;
	key2 = KEYPORT & 0x0f;
	keyval = (key1 | key2) ^ 0xff;
	Trg = keyval & (keyval ^ Cont);
	Cont = keyval;
}

u8 Key_Num(void) {
	if(Trg == 0x88) return 4;
	if(Trg == 0x84) return 5;
	if(Trg == 0x82) return 6;
	if(Trg == 0x81) return 7;
	if(Trg == 0x48) return 8;
	if(Trg == 0x44) return 9;
	if(Trg == 0x42) return 10;
	if(Trg == 0x41) return 11;
	return 0;
}

u8 code t_display[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
u8 code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码

sbit buzzer = P0^6;

typedef unsigned char BYTE;
typedef unsigned int WORD;

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define SFR */
sbit TEST_LED = P1^0;               //work LED, flash once per second

/* define variables */
WORD count;                         //1000 times counter
WORD key_count;
//-----------------------------------------------

WORD key_table[2] = {0};

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static unsigned char i;
		unsigned char read_key;
	
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
		if (key_count-- == 0) {
			key_count = 10;
			
			Read_KBD();
			read_key = Key_Num();
			if(read_key != 0) {
				key_table[0] = read_key / 10;
				key_table[1] = read_key % 10;
			}
		}
		
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 1;               //reset counter
				i++;
				if(i == 2) i = 0;
				P2 = 0xe0; P0 = ~t_display[key_table[i]]; P2 = 0x00;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0x00;
    }
}

//-----------------------------------------------

/* main program */
void main()
{
		P2 = 0xa0; buzzer = 0; P2 = 0x00;
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter

    while (1);                      //loop
}

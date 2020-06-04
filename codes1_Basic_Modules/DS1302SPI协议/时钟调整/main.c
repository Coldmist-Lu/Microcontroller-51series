#include "reg52.h"
#include "intrins.h"
#include "ds1302.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;

BYTE code T_display[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F, 0x00};
BYTE code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //Î»Âë

BYTE shi, fen, miao;

BYTE Trg, Cont;
#define KEYPORT P3

void Key_Read(void) {
	unsigned char ReadData = KEYPORT ^ 0xff;
	Trg = ReadData & (ReadData ^ Cont);
	Cont = ReadData;
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

//-----------------------------------------------

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define SFR */
//sbit TEST_LED = P1^0;               //work LED, flash once per second

/* define variables */
WORD count, key_count;                         //1000 times counter

bit key_flag;
//-----------------------------------------------
BYTE time_table[10];

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static BYTE i;
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
		if (key_count-- == 0) {
				key_count = 10;
				key_flag = 1;
		}
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 1;               //reset counter
				P2 = 0xe0; P0 = ~T_display[ time_table[i] ]; P2 = 0;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0;
				i++;
				if(i == 8) i = 0;
    }
}

//-----------------------------------------------

/* main program */

sbit buzzer = P0 ^ 6;
sbit relay = P0 ^ 4;

void main()
{
		BYTE set_shi, set_fen;
		P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0;
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter
		
		set_sfm(23, 59, 55);
		time_table[2] = 10;
		time_table[5] = 10;
    while (1) {
				EA = 0;
				shi = Ds1302_Single_Byte_Read(0x85);
				fen = Ds1302_Single_Byte_Read(0x83);
				miao = Ds1302_Single_Byte_Read(0x81); 
				EA = 1;
				time_table[0] = shi / 16;
				time_table[1] = shi % 16;
				time_table[3] = fen / 16;
				time_table[4] = fen % 16;
				time_table[6] = miao / 16;
				time_table[7] = miao % 16;
				
				if(key_flag) {
						key_flag = 0;
						Key_Read();
						if(Trg & 0x08) {
								set_fen = (fen / 16 * 10 + fen % 16 - 1 + 60) % 60;
								set_sfm(shi, set_fen, miao);
						}
						if(Trg & 0x04) {
								set_fen = (fen / 16 * 10 + fen % 16 + 1) % 60;
								set_sfm(shi, set_fen, miao);
						}
						if(Trg & 0x02) {
								set_shi = (shi / 16 * 10 + shi % 16 - 1 + 24) % 24;
								set_sfm(set_shi, fen, miao);
						}
						if(Trg & 0x01) {
								set_shi = (shi / 16 * 10 + shi % 16 + 1) % 24;
								set_sfm(set_shi, fen, miao);
						}
				}
				Delay10ms();
		}
}


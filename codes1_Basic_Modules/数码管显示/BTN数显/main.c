/* 实验1-4-2 数显+BTN */
/* by Coldmist.Lu  2019.2.18 */

#include "reg52.h"
#include "intrins.h"

#define KEYPORT P3
unsigned char Trg, Cont;

unsigned char code T_BTN[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
unsigned char code T_display[]={                       //标准字库
//   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
    0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,
//black  -     H    J    K    L    N    o   P    U     t    G    Q    r   M    y
    0x00,0x40,0x76,0x1E,0x70,0x38,0x37,0x5C,0x73,0x3E,0x78,0x3d,0x67,0x50,0x37,0x6e,
    0xBF,0x86,0xDB,0xCF,0xE6,0xED,0xFD,0x87,0xFF,0xEF,0x46};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1
unsigned char code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码

void Read_Key() {
	unsigned char KeyData = KEYPORT^0xff;
	Trg = KeyData & (KeyData ^ Cont);
	Cont = KeyData;
}

typedef unsigned char BYTE;
typedef unsigned int WORD;

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define variables */
WORD count;                         //1000 times counter

//-----------------------------------------------

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


void tm0_reset() {
		TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter
}

unsigned char cnt[8] = {0};					// COM display

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static char i;
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 2;               //reset counter
        P2 = 0xe0; P0 = ~T_display[ cnt[i] ]; P2 = 0x00;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0x00;
				if(++i == 8) i = 0;
    }
}

sbit buzzer = P0^6;

int main()
{
	unsigned char i;
	tm0_reset();
	P2 = 0xa0; buzzer = 0; P2 = 0x00;
	while(1) {
		Read_Key();
		for(i = 0; i < 8; ++i) {
			if(Trg == T_BTN[i])
				cnt[i]++;
		}
		Delay10ms();
	}
}

/* 实验1-7-2 AT24C02 */

#include "STC15F2K60S2.h"
#include "iic.h"
#include "intrins.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;


BYTE code T_display[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};    //0. 1. 2. 3. 4. 5. 6. 7. 8. 9. -1

BYTE code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码

BYTE table[8];

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

void Delay30ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 2;
	j = 67;
	k = 183;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void Delay50ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 3;
	j = 26;
	k = 223;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}


void Timer0Init(void)		//100微秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0 = 1;
	EA = 1;
}

//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static unsigned int i, count = 0;

    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 3;
				P2 = 0xe0; P0 = ~T_display[ table[i] ]; P2 = 0;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0;
				i++;
				if(i == 8) i = 0;
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
		
    Timer0Init();
		open_num = read_AT24C02(0x55);
		
//		write_AT24C02(0x55, ++open_num);
		open_num = 4;
		write_AT24C02(0x51, open_num);
		Delay50ms();
		write_AT24C02(0x52, open_num);
		Delay50ms();
		write_AT24C02(0x53, open_num);
		Delay50ms();
		write_AT24C02(0x54, open_num);
		Delay50ms();
	
		open_num = read_AT24C02(0x51);
		table[0] = open_num / 10;
		table[1] = open_num % 10;
		Delay50ms();
		open_num = read_AT24C02(0x52);
		table[2] = open_num / 10;
		table[3] = open_num % 10;
		Delay50ms();
		open_num = read_AT24C02(0x53);
		table[4] = open_num / 10;
		table[5] = open_num % 10;
		Delay50ms();
		open_num = read_AT24C02(0x54);
		table[6] = open_num / 10;
		table[7] = open_num % 10;
		Delay50ms();
		
    while (1) {
			Delay10ms();
		}
}

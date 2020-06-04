/*		实验1-1-1 利用IO关闭蜂鸣器继电器		*/
/* 实现蜂鸣器开1s关1s
   同时实现继电器关闭
   2019.1.12 by Coldmist.Lu */


#include "reg52.h"
#include "intrins.h"

sbit buzzer = P0^6;
sbit relay = P0^4;

// P25 26 27 101
// buzzer P06
// relay P04

void Delay1000ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	i = 8;
	j = 1;
	k = 243;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void main()
{
	P2 = 0xa0; buzzer = 0; P2 = 0x00;	// close the buzzer
	P2 = 0xa0; relay = 0; P2 = 0x00;	// close the relay
	while(1) 
	{
		P2 = 0xa0; buzzer = 1; P2 = 0x00;	// open the buzzer
		Delay1000ms();
		P2 = 0xa0; buzzer = 0; P2 = 0x00;	// close the buzzer
		Delay1000ms();
	}
	
}

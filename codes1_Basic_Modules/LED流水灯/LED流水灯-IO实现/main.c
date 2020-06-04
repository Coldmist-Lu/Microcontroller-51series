/*	实验1-2-1 利用IO实现LED流水灯
		2019.1.13 by Coldmist.Lu  */
#include "reg52.h"
#include "intrins.h"

sbit buzzer = P0^6;
sbit relay = P0^4;

void Delay200ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	i = 2;
	j = 103;
	k = 147;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

int main()
{
	unsigned char i = 0;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;// close the buzzer & relay
	while(1) {
		P2 = 0x80; P0 = ~(0x01 << i); P2 = 0x00;
		if(++i == 8) i = 0;
		Delay200ms();
	}
}

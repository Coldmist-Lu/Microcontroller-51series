/*  实验1-2-2 LED流水灯-MM实现
		2019.1.13 by Coldmist.Lu */

#include "reg52.h"
#include "absacc.h"
#include "intrins.h"

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


void main()
{
	unsigned char i = 0;
	XBYTE[0xA000] = 0x00;// close the buzzer and relay
	
	while(1) {
		XBYTE[0x8000] = ~(0x01 << i);
		if(++i == 8) i = 0;
		Delay200ms();
	}
	
}

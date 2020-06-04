/* 实验1-3-4 矩阵键盘实现三行代码法(自己试着写的) */
/* by coldmist.Lu  2019.2.17 */

#include "reg52.h"
#include "intrins.h"

#define KEYPORT P3

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

unsigned char Trg;
unsigned char Cont;

void Read_KBD() {
	unsigned char key1, key2;
	unsigned char ReadData;
		
	KEYPORT = 0xf0;
	key1 = KEYPORT & 0xf0;
	KEYPORT = 0x0f;
	key2 = KEYPORT & 0x0f;
	ReadData = (key1 | key2) ^ 0xff;
	Trg = ReadData & (ReadData ^ Cont);
	Cont = ReadData;
}

sbit buzzer = P0^6;

int main()
{
	P2 = 0xa0; buzzer = 0; P2 = 0x00;
	
	while(1) {
		Read_KBD();
		if(Trg == 0x11){
			P2 = 0xa0; buzzer = 1; P2 = 0x00;
		}		
		if(Trg == 0x88){
			P2 = 0xa0; buzzer = 0; P2 = 0x00;
		}				
		Delay10ms();
	}
}
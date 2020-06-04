/*	ʵ��1-3-1 �����������ԡ������д���ʵ��	*/
/* ����S4ʵ�ַ�������
	 ����S3ʵ�ַ�������
	 ���⣬ʵ���˰��¿�������������ط������Ĳ�����
   2019.1.21 by Coldmist.Lu */
#include "reg52.h"
#include "intrins.h"

#define KEYPORT P3

unsigned char Trg;
unsigned char Cont;

void Key_Read( void )
{
	unsigned char ReadData = KEYPORT^0xff;
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

sbit buzzer = P0^6;

void main()
{
	// S4
	while(1) {
		Key_Read();
		if(Trg & 0x08) { //S4
			P2 = 0xa0; buzzer = 1; P2 = 0x00;
		}
		if(Trg & 0x04) { //S3
			P2 = 0xa0; buzzer = 0; P2 = 0x00;
		}
		Delay10ms();
	}
	
	/* S4���´���������ر�
	while(1) {
		Key_Read();
		if(Cont & 0x08)//S4
		{
			P2 = 0xa0; buzzer = 1; P2 = 0x00;
		} else if(buzzer) {
			P2 = 0xa0; buzzer = 0; P2 = 0x00;
		}
	}	
	*/
}
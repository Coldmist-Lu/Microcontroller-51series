#include "STC15F2K60S2.h"
#include "intrins.h"
#include "ds1302.h"
#include "iic.h"
#include "onewire.h"

typedef unsigned char u8;
u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

unsigned char Key_Scan() {
		static unsigned char key_state = KEY_STATE_0;
		unsigned char key_value = 0, key_temp;
		u8 key1, key2;
		
		P30 = 0;P31 = 0;P32 = 0;P33 = 0;P34 = 1;P35 = 1;P42 = 1; P44 = 1;
		if( P44 == 0 ) key1 = 0x70;
		if( P42 == 0 ) key1 = 0xb0;
		if( P35 == 0 ) key1 = 0xd0;
		if( P34 == 0 ) key1 = 0xe0;
		if((P34 == 1) && (P35 == 1) && (P42 == 1) && (P44 == 1)) key1 = 0xf0;

		P30 = 1;P31 = 1;P32 = 1;P33 = 1;P34 = 0;P35 = 0;P42 = 0; P44 = 0;
		if( P33 == 0 ) key2 = 0x07;
		if( P32 == 0 ) key2 = 0x0b;
		if( P31 == 0 ) key2 = 0x0d;
		if( P30 == 0 ) key2 = 0x0e;
		if((P33 == 1) && (P32 == 1) && (P31 == 1) && (P30 == 1)) key2 = 0x0f;
		
		key_temp = key1 | key2;
		
		switch(key_state) {
				case KEY_STATE_0:
						if(key_temp != NO_KEY){
								key_state = KEY_STATE_1;
						}
						break;
				case KEY_STATE_1:
						if(key_temp == NO_KEY){
								key_state = KEY_STATE_0;
						}
						else {
								switch(key_temp)
								{
									case 0x77: key_value = 4; break;
									case 0x7b: key_value = 5; break;
									case 0x7d: key_value = 6; break;
									case 0x7e: key_value = 7; break;
									
									case 0xb7: key_value = 8; break;
									case 0xbb: key_value = 9; break;
									case 0xbd: key_value = 10; break;
									case 0xbe: key_value = 11; break;

									case 0xd7: key_value = 12; break;
									case 0xdb: key_value = 13; break;
									case 0xdd: key_value = 14; break;
									case 0xde: key_value = 15; break;

									case 0xe7: key_value = 16; break;
									case 0xeb: key_value = 17; break;
									case 0xed: key_value = 18; break;
									case 0xee: key_value = 19; break;
								}
								key_state = KEY_STATE_2;
						}
						break;
				case KEY_STATE_2: 
						if(key_temp == NO_KEY) {
							key_state = KEY_STATE_0;
						}
						break;	
		}
		return key_value;
}

void Timer0Init(void)		//1毫秒@11.0592MHz
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

bit key_flag;
sbit buzzer = P0^6;
sbit relay = P0^4;

void main(void) {
	u8 key_val = NO_KEY;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0;
	Timer0Init();
	while(1) {
		if(key_flag)
		{
			key_flag = 0;
			key_val = Key_Scan();
			switch(key_val) 
			{
				case 4: break;
				case 5: break;
				case 6: break;
				case 7: break;
				case 8: break;
				case 9: break;
				case 10: break;
				case 11: break;
				case 12: break;
				case 13: break;
				case 14: break;
				case 15: break;
				case 16: break;
				case 17: break;
				case 18: break;
				case 19: break;
			}
		}
	}
}

void Timer0() interrupt 1 using 1
{
	static int key_count = 0, smg_count = 0, i = 0;
	key_count++; smg_count++;
	if( key_count == 10 ) // 10ms
	{
		key_count = 0;
		key_flag = 1;
	}
	
	if( smg_count == 3 ) // 3ms
	{
		P2 = 0xc0; P0 = 0; P2 = 0;
		P2 = 0xe0; P0 = ~smg_du[7 - i]; P2 = 0;
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0;
		i++;
		if(i == 8) i = 0;
	}
	
}



/*		实验1-3-3 KBD测试——状态机实现		*/
/* 根据键盘开LED或关LED。
	 LED7会自己开，不知道原因是什么，带解决。
   2019.1.22 by Coldmist.Lu */
#include "reg52.h"
#include "intrins.h"

#define key_port 				P3
#define key_state_0			0		//检测按下
#define key_state_1			1		//检测抖动
#define key_state_2			2		//检测弹起		

char read_KBD(void) {
	static char key_state = 0;
	unsigned char key_press, key_return = 0;
	unsigned char key1, key2;
	
	key_port = 0xf0;
	key1 = key_port & 0xf0;
	key_port = 0x0f;
	key2 = key_port & 0x0f;
	key_press = key1 | key2;
	
	switch(key_state) {
		
		case key_state_0:
			if(key_press != 0xff) key_state = key_state_1;
			break;
		
		case key_state_1:
			if(key_press != 0xff) {
				if(key_press == 0x77) key_return = 4;
				if(key_press == 0x7b) key_return = 5;
				if(key_press == 0x7d) key_return = 6;
				if(key_press == 0x7e) key_return = 7;
				if(key_press == 0xb7) key_return = 8;
				if(key_press == 0xbb) key_return = 9;
				if(key_press == 0xbd) key_return = 10;
				if(key_press == 0xbe) key_return = 11;
				if(key_press == 0xd7) key_return = 12;
				if(key_press == 0xdb) key_return = 13;
				if(key_press == 0xdd) key_return = 14;
				if(key_press == 0xde) key_return = 15;
				if(key_press == 0xe7) key_return = 16;
				if(key_press == 0xeb) key_return = 17;
				if(key_press == 0xed) key_return = 18;
				if(key_press == 0xee) key_return = 19;
			} else {
				key_state = key_state_0;
			}
			break;
			
		case key_state_2:
			if( key_press == 0xff ) 
				key_state = key_state_0;
			break;
	}
	
	return key_return;
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

int main()
{
	
	unsigned char key_val;
	
	P2 = 0xa0; buzzer = 0; P2 = 0x00; // close the buzzer;
	
	while(1) {
		key_val = read_KBD();
		if(key_val >= 4 && key_val <= 11) {	//开LED
			P2 = 0x80; P0 = P0 & (~(0x01 << (key_val - 4))); P2 = 0x00;
		} 
		if(key_val >= 12 && key_val <= 19) {	//关LED
			P2 = 0x80; P0 = P0 | (0x01 << (key_val - 12)); P2 = 0x00;
		} 
		Delay10ms();
	}
}
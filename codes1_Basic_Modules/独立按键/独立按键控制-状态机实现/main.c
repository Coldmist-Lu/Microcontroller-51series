/*		实验1-3-1 独立按键测试——状态机实现		*/
/* 按下S7实现蜂鸣器开
	 按下S6实现蜂鸣器关
   2019.1.22 by Coldmist.Lu */
	 
#include "reg52.h"
#include "intrins.h"

#define key_input				P3
#define key_state_0 		0		//判断是否按下
#define key_state_1			1		//判断是否是抖动
#define key_state_2			2		//判断是否弹起
#define key_mask				0x0f

char read_key(void)
{
	static char key_state = 0;
	char key_press, key_return = 0;
	
	key_press = key_input & key_mask; //读入key
	
	switch(key_state) {		
		case key_state_0:
			if( key_press != key_mask ) key_state = key_state_1;
			break;
		
		case key_state_1:
			if( key_press == ( key_input & key_mask ) ) {
				if( key_press == 0x0e ) key_return = 1;	//S7
				if( key_press == 0x0d ) key_return = 2; //S6
				if( key_press == 0x0b ) key_return = 3; //S5
				if( key_press == 0x07 ) key_return = 4; //S4
				key_state = key_state_2;
			} else key_state = key_state_0;
			break;
			
		case key_state_2:
			if( key_press == 0x0f )
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

void main()
{
	unsigned char key_val;
	while(1) {
		key_val = read_key();
		if(key_val == 1) {
			P2 = 0xa0; buzzer = 1; P2 = 0x00;
		}
		if(key_val == 2) {
			P2 = 0xa0; buzzer = 0; P2 = 0x00;
		}
		Delay10ms();
	}
}

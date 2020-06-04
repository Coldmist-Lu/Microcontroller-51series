#include "STC15F2K60S2.h"
#include "intrins.h"

typedef unsigned char u8;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

bit key_flag = 0;
bit sonic_flag = 0;
u8 menu_index;
// menu0
u8 menu0_table[8];
unsigned int distance_table[4];
u8 distance_table_ptr;
// menu1
u8 menu1_table[8];
u8 data_read_ptr;
u8 data_number = 1;
// menu2
u8 menu2_table[8];
u8 last_menu;

#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

u8 Key_Scan(void) 
{
	static u8 key_state = KEY_STATE_0;
	u8 key_temp, key1, key2, key_value = 0;
	
	P30 = 0; P31 = 0; P32 = 0; P33 = 0; P34 = 1; P35 = 1; P42 = 1; P44 = 1;
	if(P44 == 0) key1 = 0x70;
	if(P42 == 0) key1 = 0xb0;
	if(P35 == 0) key1 = 0xd0;
	if(P34 == 0) key1 = 0xe0;
	if(P44 && P42 && P35 && P34) key1 = 0xf0;
	
	P30 = 1; P31 = 1; P32 = 1; P33 = 1; P34 = 0; P35 = 0; P42 = 0; P44 = 0;
	if(P33 == 0) key2 = 0x07;
	if(P32 == 0) key2 = 0x0b;
	if(P31 == 0) key2 = 0x0d;
	if(P30 == 0) key2 = 0x0e;
	if(P33 && P32 && P31 && P30) key2 = 0x0f;
	
	key_temp = key1 | key2;
	
	switch(key_state) {
		case KEY_STATE_0:
			if(key_temp != NO_KEY) {
				key_state = KEY_STATE_1;
			}
			break;
		case KEY_STATE_1:
			if(key_temp == NO_KEY) {
				key_state = KEY_STATE_0;
			} else {
				switch(key_temp) {
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
		case KEY_STATE_2:
			if(key_temp == NO_KEY) {
				key_state = KEY_STATE_0;
			}
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

void Delay5us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	i = 11;
	while (--i);
}

sbit TX = P1^0;
sbit RX = P1^1;
unsigned int sonic_distance_read(void) {
	unsigned int distance, t;
	u8 i = 8;
	do {
		TX = 1;
		Delay5us();
		TX = 0;
		Delay5us();
	} while(i--);
	TR1 = 1;
	while((RX == 1) && (TF1 == 0));
	TR1 = 0;
	
	if(TF1 == 1) {
		TF1 = 0;
		distance = 999;
	} else {
		t = TH1;
		t <<= 8;
		t |= TL1;
		distance = (unsigned int)(t * 0.017);
	}
	TH1 = 0;
	TL1 = 0;
	return distance;
}

void menu_table_init(void) 
{
	menu0_table[0] = 0x39; // C
	menu0_table[1] = 0x00;
	menu0_table[2] = smg_du[0];
	menu0_table[3] = smg_du[0];
	menu0_table[4] = smg_du[0];
	menu0_table[5] = smg_du[0];
	menu0_table[6] = smg_du[0];
	menu0_table[7] = smg_du[0];
	
	menu1_table[0] = smg_du[data_number];
	menu1_table[1] = 0x00;
	menu1_table[2] = 0x00;
	menu1_table[3] = 0x00;
	menu1_table[4] = 0x00;
	
}

sbit buzzer = P0^6;
sbit relay = P0^4;

void main()
{
	unsigned int temp_dist;
	u8 key_val = 0;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
	P2 = 0x80; P0 = 0xff; P2 = 0;
	
	menu_index = 0;
	Timer0Init();
	menu_table_init();
	distance_table_ptr = 0;
	
	while(1) {
		P2 = 0x80; P0 = ~(0x01 << menu_index); P2 = 0x00;
		if(menu_index == 1) {
			menu1_table[0] = smg_du[data_number];
			menu1_table[5] = smg_du[ distance_table[data_read_ptr] / 100 ];
			menu1_table[6] = smg_du[ distance_table[data_read_ptr] / 10 % 10 ];
			menu1_table[7] = smg_du[ distance_table[data_read_ptr] % 10 ];
		}
		if(sonic_flag) {
			sonic_flag = 0;
			temp_dist = sonic_distance_read();
			
			if(++distance_table_ptr == 4) distance_table_ptr = 0;
			distance_table[distance_table_ptr] = temp_dist;
			
			menu0_table[5] = menu0_table[2];
			menu0_table[6] = menu0_table[3];
			menu0_table[7] = menu0_table[4];
			menu0_table[2] = smg_du[temp_dist / 100];
			menu0_table[3] = smg_du[temp_dist / 10 % 10];
			menu0_table[4] = smg_du[temp_dist % 10];
			
		}
		
		if(key_flag) {
			key_flag = 0;
			key_val = Key_Scan();
			if(key_val) {
				switch(key_val) {
					case 4:
						if(menu_index == 0) {
							sonic_flag = 1;
						}
						break;
					case 5:
						if(menu_index == 0) {
							menu_index = 1;
							data_number = 1;
							data_read_ptr = distance_table_ptr == 3 ? 0 : (distance_table_ptr + 1);
						} else if(menu_index == 1) {
							menu_index = 0;
						}
						break;
					case 6:
						if(menu_index != 2) {
							last_menu = menu_index;
							menu_index = 2;
						} else {
							menu_index = last_menu;
						}
						break;
					case 7:
						if(menu_index == 1) {
							data_read_ptr++; data_number++;
							if(data_read_ptr == 4) {
								data_read_ptr = 0;
							}
							if(data_number == 5) {
								data_number = 1;
							}
						}
						break;
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
					case 18:
						P2 = 0xa0; buzzer = 1; P2 = 0x00;
						break;
					case 19:
						P2 = 0xa0; buzzer = 0; P2 = 0x00;
						break;
				}
			}
		}
	}
}

void Timer0(void) interrupt 1 using 1
{
	static int i = 0, key_cnt = 0, smg_cnt = 0;
	key_cnt++; smg_cnt++;
	
	if(key_cnt == 10) {
		key_cnt = 0;
		key_flag = 1;
	}
	
	if(smg_cnt == 3) {
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0x00;
		if(menu_index == 0) {
			P2 = 0xe0; P0 = ~menu0_table[i]; P2 = 0x00;
		}
		if(menu_index == 1) {
			P2 = 0xe0; P0 = ~menu1_table[i]; P2 = 0x00;
		}
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0x00;
		i++;
		if(i == 8) i = 0;
	}
}




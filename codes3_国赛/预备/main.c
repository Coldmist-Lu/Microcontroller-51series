#include "STC15F2K60S2.h"
#include "intrins.h"
#include "onewire.h"
#include "iic.h"

typedef unsigned char u8;
typedef unsigned int u16;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

#define KEY_PORT P3
#define NO_KEY 0x0f
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

u8 key_state = KEY_STATE_0;
u8 key_val = 0;
u8 key_val_cont = 0;
u8 adc_val = 0;
u8 table[8];

float temperature;

bit key_flag = 0;
bit temperature_flag = 0;
bit adc_flag = 1;

sbit buzzer = P0^6;
sbit relay = P0^4;


u8 read_btn(void)
{
	u8 key_temp, key_value = 0;
	P30 = 1; P31 = 1; P32 = 1; P33 = 1;
	if(P33 == 0) key_temp = 0x07;
	if(P32 == 0) key_temp = 0x0b;
	if(P31 == 0) key_temp = 0x0d;
	if(P30 == 0) key_temp = 0x0e;
	if(P33 && P32 && P31 && P30) key_temp = 0x0f;
	
	switch(key_state)
	{
		case KEY_STATE_0:
			key_val_cont = 0;
			if(key_temp != NO_KEY) {
				key_state = KEY_STATE_1;
			}
			break;
		case KEY_STATE_1:
			if(key_temp == NO_KEY)
				key_state = KEY_STATE_0;
			else {
				switch(key_temp) {
					case 0x07: key_value = 4; break;
					case 0x0b: key_value = 5; break;
					case 0x0d: key_value = 6; break;
					case 0x0e: key_value = 7; break;				
				}
				key_state = KEY_STATE_2;
				key_val_cont = key_value;
			}
			break;
		case KEY_STATE_2:
			if(key_temp == NO_KEY)
				key_state = KEY_STATE_0;
			break;			
	}
	return key_value;
}

//u8 read_key(void)
//{
//	u8 key_temp, key1, key2, key_value = 0;
//	
//	P30=0; P31=0; P32=0; P33=0; P34=1; P35=1; P42=1; P44=1;
//	if(P44 == 0) key1 = 0x70;
//	if(P42 == 0) key1 = 0xb0;
//	if(P35 == 0) key1 = 0xd0;
//	if(P34 == 0) key1 = 0xe0;
//	if(P34==1 && P35==1 && P42==1 && P44==1) key1 = 0xf0;
//	
//	P30=1; P31=1; P32=1; P33=1; P34=0; P35=0; P42=0; P44=0;
//	if(P33 == 0) key2 = 0x07;
//	if(P32 == 0) key2 = 0x0b;
//	if(P31 == 0) key2 = 0x0d;
//	if(P30 == 0) key2 = 0x0e;
//	if(P30==1 && P31==1 && P32==1 && P33==1) key2 = 0x0f;
//	
//	key_temp = key1 | key2;
//	
//	switch(key_state)
//	{
//		case KEY_STATE_0:
//			key_val_cont = 0;
//			if(key_temp != NO_KEY) {
//				key_state = KEY_STATE_1;
//			}
//			break;
//		case KEY_STATE_1:
//			if(key_temp == NO_KEY)
//				key_state = KEY_STATE_0;
//			else {
//				switch(key_temp) {
//					case 0x77: key_value = 4; break;
//					case 0x7b: key_value = 5; break;
//					case 0x7d: key_value = 6; break;
//					case 0x7e: key_value = 7; break;
//					case 0xb7: key_value = 8; break;
//					case 0xbb: key_value = 9; break;
//					case 0xbd: key_value = 10; break;
//					case 0xbe: key_value = 11; break;
//					case 0xd7: key_value = 12; break;
//					case 0xdb: key_value = 13; break;
//					case 0xdd: key_value = 14; break;
//					case 0xde: key_value = 15; break;	
//					case 0xe7: key_value = 16; break;
//					case 0xeb: key_value = 17; break;
//					case 0xed: key_value = 18; break;
//					case 0xee: key_value = 19; break;						
//				}
//				key_state = KEY_STATE_2;
//				key_val_cont = key_value;
//			}
//			break;
//		case KEY_STATE_2:
//			if(key_temp == NO_KEY)
//				key_state = KEY_STATE_0;
//			break;			
//	}
//	
//	return key_value;
//}

void Timer0Init(void)		//1毫秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	EA = 1;
	ET0 = 1;
}

void key_trig_f(void)
{
	if(key_val == 0) return;
	switch(key_val) {
		case 4: P2 = 0xa0; buzzer = 1; P2 = 0x00; break;
		case 5: P2 = 0xa0; buzzer = 0; P2 = 0x00; break;
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

void main()
{
	P2 = 0xa0; P0 = 0x00; P2 = 0x00;
	P2 = 0x80; P0 = 0xff; P2 = 0x00;
	Timer0Init();
	while(1)
	{
		if(key_flag)
		{
			key_flag = 0;
			key_val = read_btn();
			if(key_val) key_trig_f();
		}
//		if(temperature_flag)
//		{
//			temperature_flag = 0;
//			temperature = rd_temperature_f();
//			table[0] = smg_du[((u8)temperature / 10)];
//			table[1] = smg_du[((u8)temperature % 10)] | 0x80;
//			table[2] = smg_du[(u8)(temperature * 10) % 10];
//		}
		if(adc_flag)
		{
			adc_flag = 0;
			adc_val = read_adc(0x03);
			table[0] = smg_du[adc_val / 100];
			table[1] = smg_du[adc_val / 10 % 10];
			table[2] = smg_du[adc_val % 10];
		}
		
	}
}

void T0_int() interrupt 1 using 1
{
	static u16 key_cnt = 0, smg_cnt = 0, i = 0;
	static u16 tpr_cnt = 0, adc_cnt = 0;
	
	if(++key_cnt == 10) {
		key_cnt = 0;
		key_flag = 1;
	}
	if(++smg_cnt == 3) {
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0x00;
		P2 = 0xe0; P0 = ~table[i]; P2 = 0x00;
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0x00;
		if(++i == 8) i = 0;
	}
	if(++tpr_cnt == 200) {
		tpr_cnt = 0;
		temperature_flag = 1;
	}
	if(++adc_cnt == 200) {
		adc_cnt = 0;
		adc_flag = 1;
	}
}
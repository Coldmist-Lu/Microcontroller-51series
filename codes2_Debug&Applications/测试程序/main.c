#include "STC15F2K60S2.h"
#include "intrins.h"
#include "iic.h"
#include "ds1302.h"
#include "onewire.h"

typedef unsigned char u8;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

bit key_flag = 0;
bit led_flag = 0;
bit adc_flag = 0;

u8 smg_adc[8];
u8 smg_eeprom[8];
u8 smg_time[8];
u8 smg_temp[8];
u8 smg_dist[8];

#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

u8 key_state = KEY_STATE_0;

u8 Key_Scan(void)
{
	u8 key_temp, key_value = 0;
	u8 key1, key2;
	
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
	
	switch(key_state)
	{
		case KEY_STATE_0:
			if(key_temp != NO_KEY)
				key_state = KEY_STATE_1;
			break;
		case KEY_STATE_1:
			if(key_temp == NO_KEY)
				key_state = KEY_STATE_0;
			else
			{
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
			if(key_temp == NO_KEY)
				key_state = KEY_STATE_0;
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

void Delay9us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 22;
	while (--i);
}


void Delay100ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 5;
	j = 52;
	k = 195;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void Ds1302_routine(void)
{
	u8 shi, fen, miao;
	shi = Read_Ds1302_Byte(0x85);
	fen = Read_Ds1302_Byte(0x83);
	miao = Read_Ds1302_Byte(0x81);
	smg_time[0] = smg_du[shi / 16];
	smg_time[1] = smg_du[shi % 16];
	smg_time[3] = smg_du[fen / 16];
	smg_time[4] = smg_du[fen % 16];
	smg_time[6] = smg_du[miao / 16];
	smg_time[7] = smg_du[miao % 16];
}

sbit TX = P1^0;
sbit RX = P1^1;

unsigned int sonic_distance(void)
{
	unsigned char i = 8;
	unsigned int distance, t;
	do
	{
		TX = 1;
		Delay9us();
		TX = 0;
		Delay9us();
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


sbit buzzer = P0^6;
sbit relay = P0^4;

void main(void)
{
	u8 key_val = 0;
	u8 adc_val = 0;
	u8 eeprom_val = 0;
	float temperature = 0;
	unsigned int distance = 999;
	
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
	P2 = 0x80; P0 = 0xff; P2 = 0x00;
	
	eeprom_val = read_eeprom(0x51);
	smg_eeprom[0] = smg_du[eeprom_val / 10];
	smg_eeprom[1] = smg_du[eeprom_val % 10];
	
	eeprom_val = read_eeprom(0x52);
	smg_eeprom[3] = smg_du[eeprom_val / 10];
	smg_eeprom[4] = smg_du[eeprom_val % 10];
	
	eeprom_val = read_eeprom(0x53);		
	smg_eeprom[6] = smg_du[eeprom_val / 10];
	smg_eeprom[7] = smg_du[eeprom_val % 10];

	set_sfm(23, 59, 55);
	
	Timer0Init();
	while(1)
	{
		Ds1302_routine();
		if(adc_flag)
		{
			adc_flag = 0;
			write_adc(0x01);
			adc_val = read_adc(0x01);
			smg_adc[0] = smg_du[adc_val / 100];
			smg_adc[1] = smg_du[adc_val / 10 % 10];
			smg_adc[2] = smg_du[adc_val % 10];
		}
			
		if(led_flag)
		{
			P2 = 0x80; P0 = ~0x01; P2 = 0x00;
		}
		else 
		{
			P2 = 0x80; P0 = ~0x00; P2 = 0x00;
		}
		if(key_flag)
		{
			key_flag = 0;
			key_val = Key_Scan();
			if(key_val) {
				switch(key_val) {
					case 4:
						led_flag = ~led_flag;
						break;
					case 5: 
						eeprom_val = read_eeprom(0x51);
						write_eeprom(0x51, ++eeprom_val);
						smg_eeprom[0] = smg_du[eeprom_val / 10];
						smg_eeprom[1] = smg_du[eeprom_val % 10];
						break;
					case 6: 
						eeprom_val = read_eeprom(0x52);
						write_eeprom(0x52, ++eeprom_val);
						smg_eeprom[3] = smg_du[eeprom_val / 10];
						smg_eeprom[4] = smg_du[eeprom_val % 10];
						break;
					case 7:
						eeprom_val = read_eeprom(0x53);
						write_eeprom(0x53, ++eeprom_val);
						smg_eeprom[6] = smg_du[eeprom_val / 10];
						smg_eeprom[7] = smg_du[eeprom_val % 10];
						break;
					case 8: break;
					case 9: break;
					case 10:
						distance = sonic_distance();
						smg_dist[0] = smg_du[distance / 100];
						smg_dist[1] = smg_du[distance / 10 % 10];
						smg_dist[2] = smg_du[distance % 10];
						break;
					case 11: 
						temperature = read_temperature();
						smg_temp[0] = smg_du[(unsigned char)temperature / 10];
						smg_temp[1] = smg_du[(unsigned char)temperature % 10] | 0x80;
						smg_temp[2] = smg_du[(unsigned char)(temperature * 10) % 10];
						break;
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
}

void Timer0(void) interrupt 1 using 1
{
	static unsigned int i = 0, key_cnt = 0, smg_cnt = 0;
	static unsigned int adc_cnt = 0;
	key_cnt++, smg_cnt++;
	adc_cnt++;
	
	if(adc_cnt == 100)
	{
		adc_cnt = 0;
		adc_flag = 1;
	}
	
	if(key_cnt == 10)
	{
		key_cnt = 0;
		key_flag = 1;
	}
	
	if(smg_cnt == 3)
	{
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0x00;
		P2 = 0xe0; P0 = ~smg_dist[i]; P2 = 0x00;
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0x00;
		i++;
		if(i == 8) i = 0;
	}
}
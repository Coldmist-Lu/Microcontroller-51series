#include "STC15F2K60S2.h"
#include "intrins.h"

unsigned char code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
unsigned char code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

sbit buzzer = P0^6;
sbit relay = P0^4;
bit sonic_flag = 0;
unsigned int t, distance = 999;
unsigned char table[8];


void Delay9us()		//@11.0592MHz
{
	unsigned char i;

	_nop_();
	_nop_();
	i = 22;
	while (--i);
}

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


sbit TX = P1^0;
sbit RX = P1^1;

void send_wave(void)
{
	unsigned char i = 8;
	do {
		TX = 1;
		Delay9us();
		TX = 0;
		Delay9us();
	} while(i--);
}

void distance_update(void)
{
	TH1 = 0;
	TL1 = 0;
	send_wave();
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
}


void main()
{
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
	P2 = 0x80; P0 = 0xff; P2 = 0x00;
	TH1 = 0; TL1 = 0;
	Timer0Init();
	while(1) {
		if(sonic_flag) {
			sonic_flag = 0;
			distance_update();
			table[0] = smg_du[distance / 100];
			table[1] = smg_du[distance / 10 % 10];
			table[2] = smg_du[distance % 10];
		}
	}
}

void Timer0(void) interrupt 1 using 1
{
	static unsigned int i = 0, smg_cnt = 0, sonic_cnt = 0;
	smg_cnt++;
	if(smg_cnt == 3)
	{
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0x00;
		P2 = 0xe0; P0 = ~table[i]; P2 = 0x00;
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0x00;
		i++;
		if(i == 8) i = 0;
	}
	sonic_cnt++;
	if(sonic_cnt == 200)
	{
		sonic_flag = 1;
		sonic_cnt = 0;
	}
}





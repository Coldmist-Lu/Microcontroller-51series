#include "STC15F2K60S2.h"
#include "intrins.h"
#include "iic.h"
#include "ds1302.h"

typedef unsigned char u8;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

bit key_flag = 0;			// keyboard_read
bit set_sd_flag = 0;	// set humidity threshold mode
bit mode;							// set mode0: automatic 	mode1: manual 
bit sd_lim_flag = 0;	// to show if system should start or not
bit irrigater_open_flag = 0;	// to show system start
bit alert_flag = 0;		// to show if alert on/off
u8 sd_lim = 50;				// humidity threshold
u8 time_table[8];			// table1: if not setting threshold
u8 set_table[8];			// table2: if setting threshold

#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

u8 Key_Scan(void) // key_scanning function
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
	
	switch(key_state) 
	{
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

void Display_time(void) { // 读DS1302并显示
	u8 shi, fen, miao;
	shi = Ds1302_Single_Byte_Read(0x85);
	fen = Ds1302_Single_Byte_Read(0x83);
	miao = Ds1302_Single_Byte_Read(0x81);
	time_table[0] = smg_du[shi / 16];
	time_table[1] = smg_du[shi % 16];
	time_table[3] = smg_du[fen / 16];
	time_table[4] = smg_du[fen % 16];
}

void smg_table_init(void) { // 数码管表的初始化（不变量初始化）
	time_table[2] = 0x40;
	time_table[5] = 0x00;
	set_table[0] = 0x40;
	set_table[1] = 0x40;
	set_table[2] = 0x00;
	set_table[3] = 0x00;
	set_table[4] = 0x00;
	set_table[5] = 0x00;
}

sbit buzzer = P0^6;
sbit relay = P0^4;

void main()
{
	unsigned int sd;
	u8 key_val = 0;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
	P2 = 0x80; P0 = 0xff; P2 = 0;
	
	Timer0Init();   // 初始化定时器
	smg_table_init();  // 初始化数码管不变量
	
	alert_flag = 1;	// 开启报警功能
	mode = 0;				// 默认模式0：自动
 	set_sfm(8, 30, 0); // 设置DS1302时钟
	
	while(1)
	{
		Display_time(); // 读取DS1302时钟并显示
		
		if(set_sd_flag) // 如果是设置阈值模式
		{ // 如有必要，修改数码管的值
			set_table[6] = smg_du[sd_lim / 10];
			set_table[7] = smg_du[sd_lim % 10];
		} 
		else // 正常模式 
		{ // 读取湿度（adc）
			write_adc(0x03);
			sd = (unsigned int)(read_adc(0x03) * 99) / 255;
			// 如低于指定阈值，修改标记
			sd_lim_flag = sd < sd_lim ? 1 : 0;
			// 修改湿度
			time_table[6] = smg_du[(u8)(sd / 10)];
			time_table[7] = smg_du[(u8)(sd % 10)];
		}
		
		if(mode == 0) { // 自动模式
			P2 = 0x80; P0 = ~0x01; P2 = 0; // LED control
			if(sd_lim_flag) { // 根据阈值开关relay
				P2 = 0xa0; buzzer = 0; relay = 1; P2 = 0x00;
				irrigater_open_flag = 1;
			} else {
				P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
				irrigater_open_flag = 0;
			}
		}
		else 
		{ // 手动模式
			P2 = 0x80; P0 = ~0x02; P2 = 0; // LED control
			if(sd_lim_flag && alert_flag) { // 如果低于阈值，并且警报标志打开
				P2 = 0xa0; buzzer = 1; P2 = 0x00;
			} else {
				P2 = 0xa0; buzzer = 0; P2 = 0x00;
			}
			if(irrigater_open_flag) { // 否则，如果手动开启
				P2 = 0xa0; relay = 1; P2 = 0x00;
			} else {
				P2 = 0xa0; relay = 0; P2 = 0x00;
			}
		}
		
		if(key_flag) { // 如果读key
			key_flag = 0;
			key_val = Key_Scan();
			if(key_val) {
				switch(key_val) {
					case 4:
						if(mode == 1) { // 手动模式，关system
							irrigater_open_flag = 0;
						} else { // 自动模式，如果在设置状态，降低阈值
							if(set_sd_flag) {
								sd_lim--;
							}
						}
						break;
					case 5:
						if(mode == 1) { // 手动模式，开system
							irrigater_open_flag = 1;
						} else { // 自动模式，如果在设置状态，提高阈值
							if(set_sd_flag) { 
								sd_lim++;
							}
						}
						break;
					case 6:
						if(mode == 1) { // 手动模式，开关警报器
							alert_flag = !alert_flag;
						} else { // 自动模式，开设置或关设置并写EEPROM
							if(set_sd_flag) {
								write_AT24C02(0x03, sd_lim);
								set_sd_flag = 0;
							} else {
								set_sd_flag = 1;
							}
						}
						break;
					case 7:
						mode = !mode;	// 工作状态切换
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
					case 18: break;
					case 19: break;
				}
			}
		}
	}
}

void Timer0(void) interrupt 1 using 1 // 中断程序
{
	static unsigned int key_cnt = 0, smg_cnt = 0, i = 0;

	key_cnt++; smg_cnt++;
	if(key_cnt == 10) { //10ms读一次key
		key_cnt = 0;
		key_flag = 1;
	}
	if(smg_cnt == 3) { //3ms刷新一个数码管
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0x00;
		if(set_sd_flag) {
			P2 = 0xe0; P0 = ~set_table[i]; P2 = 0x00;
		} else {
			P2 = 0xe0; P0 = ~time_table[i]; P2 = 0x00;			
		}
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0x00;
		i++;
		if(i == 8) i = 0;
	}
}


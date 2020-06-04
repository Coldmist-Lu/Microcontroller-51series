#include "STC15F2K60S2.h"
#include "intrins.h"
#include "iic.h"

#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

typedef unsigned char u8;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
u8 code led_mode1[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};
u8 code led_mode2[] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
u8 code led_mode3[] = {0x81,0x42,0x24,0x18};
u8 code led_mode4[] = {0x18,0x24,0x42,0x81};


bit LED_flag = 0;
bit key_flag = 0;
bit bling_start_flag = 0;
bit mode_change_flag = 0;
bit adc_read_flag = 0;
bit show_lighting_mode_flag = 0;

u8 mode_index;
unsigned int delay_time_table[] = {0, 400, 400, 400, 400};
u8 setting_table[8];
u8 setting_mode;
u8 adc_port_val;
u8 lighting_mode = 0;
u8 led_val;
u8 pwm_table[5] = {10, 9, 6, 3, 0}; // 第一位不要，亮度最暗是1，最亮是10
u8 lighting_table[8];

u8 key_state = KEY_STATE_0;

u8 Key_Scan(void) {
	u8 key_temp, key_value = 0, key1, key2;
	
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
			break;
	  case KEY_STATE_2:
			if(key_temp == NO_KEY) {
				key_state = KEY_STATE_0;
			}
			break;
	}
	
	return key_value;
}

void Timer0Init(void)		//1ms@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xCD;		//设置定时初值
	TH0 = 0xD4;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计
	ET0 = 1;
	EA = 1;
}

void Delay30ms()		//@11.0592MHz
{
	unsigned char i, j, k;

	_nop_();
	_nop_();
	i = 2;
	j = 67;
	k = 183;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void Table_init(void) {
	unsigned char tmp = 4;
	setting_table[0] = 0x40;
	setting_table[2] = 0x40;
	setting_table[3] = 0x00;
	lighting_table[0] = 0x00;
	lighting_table[1] = 0x00;
	lighting_table[2] = 0x00;
	lighting_table[3] = 0x00;
	lighting_table[4] = 0x00;
	lighting_table[5] = 0x00;
	lighting_table[6] = 0x40;
	
	delay_time_table[1] = read_AT24C02(0x51) * 100;
	Delay30ms();
	delay_time_table[2] = read_AT24C02(0x52) * 100;
	Delay30ms();
	delay_time_table[3] = read_AT24C02(0x53) * 100;
	Delay30ms();
	delay_time_table[4] = read_AT24C02(0x54) * 100;
	Delay30ms();
}

sbit buzzer = P0^6;
sbit relay = P0^4;
void main(void)
{
	u8 key_val = 0;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
	P2 = 0x80; P0 = 0xff; P2 = 0x00;
	
	
	Timer0Init();
	mode_index = 3;
	Table_init();		// 数码管表初始化
	setting_mode = 0; // 设置模式初始化（未进入设置）
	
	adc_read_flag = 1;
	led_val = 0xff;
	
	while(1)
	{
		if(adc_read_flag) {
			adc_read_flag = 0;
			write_adc(0x03); // CH3
			adc_port_val = read_adc(0x03);
			if(adc_port_val < 64) lighting_mode = 1;
			else if(adc_port_val < 128) lighting_mode = 2;
			else if(adc_port_val < 192) lighting_mode = 3;
			else lighting_mode = 4;
		}
		
		if(setting_mode == 1 || setting_mode == 2) {
			setting_table[1] = smg_du[mode_index];
			
			if(delay_time_table[mode_index] > 999)
				setting_table[4] = smg_du[1];
			else 
				setting_table[4] = 0x00;
			
			setting_table[5] = smg_du[delay_time_table[mode_index] / 100 % 10];
			setting_table[6] = smg_du[0];
			setting_table[7] = smg_du[0];
			bling_start_flag = 1;
		} else
		{
			bling_start_flag = 0;
			if(show_lighting_mode_flag) {
				lighting_table[7] = smg_du[lighting_mode];
				if(key_state == KEY_STATE_0) {
					show_lighting_mode_flag = 0;
				}
			}
		}
		
		if(key_flag) {
			key_flag = 0;
			key_val = Key_Scan();
			if(key_val) {
				switch(key_val) {
		 			case 4:
						if(setting_mode == 1) {
							mode_index--;
							if(mode_index == 0) mode_index = 4;
							mode_change_flag = 1;
						} else if(setting_mode == 2) {
							delay_time_table[mode_index] -= 100;
							if(delay_time_table[mode_index] < 400)
								delay_time_table[mode_index] = 1200;
						} else if(setting_mode == 0) {
							show_lighting_mode_flag = 1;
						}
						break;
					case 5:
						if(setting_mode == 1) {
							mode_index++;
							if(mode_index == 5) mode_index = 1;
							mode_change_flag = 1;
						} else if(setting_mode == 2) {
							delay_time_table[mode_index] += 100;
							if(delay_time_table[mode_index] > 1200)
								delay_time_table[mode_index] = 400;
						}
						break;
					case 6:
						setting_mode++;
						if(setting_mode == 3) {
							write_AT24C02((0x30 + mode_index), (delay_time_table[mode_index] / 100));
							setting_mode = 0;
						}
						break;
					case 7:
						LED_flag = ~LED_flag;
						break;
					case 8: 
						break;
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
						P2 = 0xa0; buzzer = 0; relay = 1; P2 = 0x00;
						break;
					case 19: 
						P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00;
						break;
				}
			}
		}
	}
}

void Timer0(void) interrupt 1 using 1
{
	static int smg_cnt = 0, key_cnt = 0, i = 0;	
	static int led_cnt = 0, led_idx = 0, bling_cnt = 0;
	static int adc_read_cnt = 0, pwm_cnt = 0;
	static bit bling_flag = 1;	// 1 为显示
	
	if(mode_change_flag) {
		mode_change_flag = 0;
		led_idx = 0;
		led_cnt = delay_time_table[mode_index] - 1; // 准备重新显示
	}
	
	if(LED_flag) 
	{
		led_cnt++;
		if(led_cnt == delay_time_table[mode_index]) 
		{
			led_cnt = 0;
			switch(mode_index) {
				case 1:
					if(led_idx == 8) led_idx = 0;
					led_val = ~led_mode1[led_idx];
					P2 = 0x80; P0 = ~led_mode1[led_idx]; P2 = 0;
					led_idx++;
					break;
				case 2:
					if(led_idx == 8) led_idx = 0;
					led_val = ~led_mode2[led_idx];
					P2 = 0x80; P0 = ~led_mode2[led_idx]; P2 = 0;
					led_idx++;
					break;
				case 3:
					if(led_idx == 4) led_idx = 0;
					led_val = ~led_mode3[led_idx];
					P2 = 0x80; P0 = ~led_mode3[led_idx]; P2 = 0;
					led_idx++;
					break;
				case 4:
					if(led_idx == 4) led_idx = 0;
					led_val = ~led_mode4[led_idx];
					P2 = 0x80; P0 = ~led_mode4[led_idx]; P2 = 0;
					led_idx++;
					break;
			}
		}
	}
	
	if(bling_start_flag) {
		bling_cnt++;
		if(bling_cnt == 800) {
			bling_cnt = 0;
			bling_flag = ~bling_flag;
		}
	}
	
	adc_read_cnt++;
	if(adc_read_cnt == 200) { // 200ms
		adc_read_cnt = 0;
		adc_read_flag = 1;
	}
	
	if(lighting_mode != 4) {
		pwm_cnt++; // 控制亮度
		if(pwm_cnt >= pwm_table[lighting_mode]) {
			P2 = 0x80; P0 = led_val; P2 = 0x00;
		}
		if(pwm_cnt >= 10) {
			P2 = 0x80; P0 = 0xff; P2 = 0x00;
			pwm_cnt = 0;
		}
	}
	
	smg_cnt++; key_cnt++;
	if(smg_cnt == 3) {
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0x00;
		if(setting_mode) { // 设置模式闪烁配置
			if(bling_flag) {
				P2 = 0xe0; P0 = ~setting_table[i]; P2 = 0x00;
			} else {
				if(setting_mode == 1) {
					if(i < 3) { P2 = 0xe0; P0 = ~0x00; P2 = 0x00; }
					else { P2 = 0xe0; P0 = ~setting_table[i]; P2 = 0x00; }
				}
				if(setting_mode == 2) {
					if(i > 3) { P2 = 0xe0; P0 = ~0x00; P2 = 0x00; }
					else { P2 = 0xe0; P0 = ~setting_table[i]; P2 = 0x00; }
				}
			}
		} else {
			if(show_lighting_mode_flag) {
				P2 = 0xe0; P0 = ~lighting_table[i]; P2 = 0x00;
			} else {
				P2 = 0xe0; P0 = ~0x00; P2 = 0x00;
			}
		}
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0x00;
		i++;
		if(i == 8) i = 0;
	}
	

	if(key_cnt == 10) {
		key_cnt = 0;
		key_flag = 1;
	}
}

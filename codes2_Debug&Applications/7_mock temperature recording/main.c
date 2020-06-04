#include "STC15F2K60S2.h"
#include "ds1302.h"
#include "onewire.h"
#include "intrins.h"

typedef unsigned char u8;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

u8 menu_index;
//menu0
u8 menu0_table[8];
u8 sample_select[4] = {1, 5, 30, 60};
u8 sample_ptr;
//menu1
u8 temp_time = 1;				// 温度采集间隔时间
u8 menu1_table[8];			// 数码管显示数组
float temp_table[10];		// 温度数据
u8 temp_table_ptr;
//menu2
u8 menu2_table[8];
u8 temp_index_ptr;



#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

u8 Key_Scan(void) 
{
	u8 key_state, key_value = 0, key_temp;
	u8 key1, key2;
	
	P30 = 0; P31 = 0; P32 = 0; P33 = 0; P34 = 1; P35 = 1; P42 = 1; P44 = 1; 
	if(P44 == 0) key1 = 0x70;
	if(P42 == 0) key1 = 0xb0;
	if(P35 == 0) key1 = 0xd0;
	if(P34 == 0) key1 = 0xe0;
	if((P44 == 1) && (P42 == 1) && (P35 == 1) && (P34 == 1)) key1 = 0xf0;
	
	P30 = 1; P31 = 1; P32 = 1; P33 = 1; P34 = 0; P35 = 0; P42 = 0; P44 = 0; 
	if(P33 == 0) key2 = 0x07;
	if(P32 == 0) key2 = 0x0b;
	if(P31 == 0) key2 = 0x0d;
	if(P30 == 0) key2 = 0x0e;
	if((P33 == 1) && (P32 == 1) && (P31 == 1) && (P30 == 1)) key2 = 0x0f;
	
	key_temp = key1 | key2;
	
	switch(key_state) {
		case KEY_STATE_0:
			if(key_temp != NO_KEY)
				key_state = 1;
			break;
			
		case KEY_STATE_1:
			if(key_temp == NO_KEY)
				key_state = 0;
			else {
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

void menu0_init(void) {
	menu_index = 0;
	menu0_table[0] = 0x00;
	menu0_table[1] = 0x00;
	menu0_table[2] = 0x00;
	menu0_table[3] = 0x00;
	menu0_table[4] = 0x00;
	menu0_table[5] = 0x40;
	menu0_table[6] = smg_du[sample_select[0] / 10];
	menu0_table[7] = smg_du[sample_select[0] % 10];
	sample_ptr = 0;
}

void menu1_routine(void) {
	u8 shi, fen, miao;
	shi = Ds1302_Single_Byte_Read(0x85);
	fen = Ds1302_Single_Byte_Read(0x83);
	miao = Ds1302_Single_Byte_Read(0x81);
	menu1_table[0] = smg_du[shi / 16];
	menu1_table[1] = smg_du[shi % 16];
	menu1_table[3] = smg_du[fen / 16];
	menu1_table[4] = smg_du[fen % 16];
	menu1_table[6] = smg_du[miao / 16];
	menu1_table[7] = smg_du[miao % 16];
}

void menu2_init(void) 
{
  temp_index_ptr = 0;
  menu2_table[0] = 0x40;
  menu2_table[1] = smg_du[temp_index_ptr / 10];
  menu2_table[2] = smg_du[temp_index_ptr % 10];
  menu2_table[3] = 0x00;
  menu2_table[4] = 0x00;
  menu2_table[5] = 0x40;
  menu2_table[6] = smg_du[(unsigned char)(temp_table[temp_index_ptr]) / 10];
  menu2_table[7] = smg_du[(unsigned char)(temp_table[temp_index_ptr]) % 10];
}

bit key_flag;
bit tip_toggle_flag;
bit temp_flag;
bit LED_toggle_flag;
bit LED_flag;

sbit buzzer = P0^6;
sbit relay = P0^4;

void main(void)
{
	u8 key_val;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00; // close buzzer & relay
  P2 = 0x80; P0 = 0xff; P2 = 0x00; // close LED
	Timer0Init();
	
	menu0_init(); // show menu0
	set_sfm(23, 59, 50); // set DS1302
	while(1) 
	{
		if(menu_index == 1) { // menu_1 routine
			menu1_routine(); // read DS1302
			if(temp_flag == 1) { // read DS18B20
				temp_flag = 0;
				EA = 0;
				temp_table[temp_table_ptr] = read_temperature_f();
				EA = 1;
				temp_table_ptr++;
				if(temp_table_ptr == 10) { // 温度采集完毕
					menu_index = 2;
          LED_flag = 1;
          menu2_init();
				}
			}
		}
		if(key_flag == 1) { // read key
			key_flag = 0;
			key_val = Key_Scan();
			if(key_val) {
				switch(key_val) {
					case 4:
            if(menu_index == 0) { // only function in menu0
              sample_ptr++;
              if(sample_ptr == 4) sample_ptr = 0;
              menu0_table[6] = smg_du[sample_select[sample_ptr] / 10];
              menu0_table[7] = smg_du[sample_select[sample_ptr] % 10];              
            }
            break;
					case 5: 
            if(menu_index == 0) { // only function in menu0
              menu_index = 1;
              temp_time = sample_select[sample_ptr];
              temp_table_ptr = 0;
            }
            break;
					case 6: 
            if(menu_index == 2) { // only function in menu2
              LED_flag = 0;
              P2 = 0x80; P0 = 0xff; P2 = 0;
              temp_index_ptr++;
              if(temp_index_ptr == 10) {
                temp_index_ptr = 0;
              }
              menu2_table[1] = smg_du[temp_index_ptr / 10];
              menu2_table[2] = smg_du[temp_index_ptr % 10];
              menu2_table[6] = smg_du[(unsigned char)(temp_table[temp_index_ptr]) / 10];
              menu2_table[7] = smg_du[(unsigned char)(temp_table[temp_index_ptr]) % 10];
            }
            break;
					case 7: 
            if(menu_index == 2) { // only function in menu2
              menu_index = 0;
              menu0_init();
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
					case 18: break;
					case 19: break;
				}
			}
		}
	}
}

void Timer0() interrupt 1 using 1
{
	static int key_count = 0, smg_count = 0, i = 0;
	static int tip_count = 0, temp_count = 0, LED_count = 0;
		
	key_count++; smg_count++; 
	if(key_count == 10) { // 10ms 读取 keyboard
		key_count = 0;
		key_flag = 1;
	}
  if(LED_flag) { // LED 闪烁标志
    LED_count++;
    if(LED_count == 1000) { // 每隔 1s 闪烁一次
      LED_count = 0;
      LED_toggle_flag = ~LED_toggle_flag;
      if(LED_toggle_flag) {
        P2 = 0x80; P0 = ~0x01; P2 = 0x00;
      } else {
        P2 = 0x80; P0 = ~0x00; P2 = 0x00;        
      }
    }
  }
  
	if(menu_index == 1) {
		tip_count++; temp_count++; 
		if(tip_count == 1000) { //  每隔1s标志位闪烁一次
			tip_count = 0;
			tip_toggle_flag = ~tip_toggle_flag;
			if(tip_toggle_flag) {
				menu1_table[2] = 0x40;
				menu1_table[5] = 0x40;
			} else {
				menu1_table[2] = 0x00;
				menu1_table[5] = 0x00;
			}
		}
    if(temp_count == 1000 * temp_time) { // 每隔一个采样周期（默认1s）采集一次温度
      temp_count = 0;
      temp_flag = 1;
    }
	}
	
	if(smg_count == 3) { // 每隔3ms刷新一个数码管
		smg_count = 0;
		P2 = 0xc0; P0 = 0x00; P2 = 0;
		
		if(menu_index == 0) {
			P2 = 0xe0; P0 = ~menu0_table[i]; P2 = 0;
		}
		if(menu_index == 1) {
			P2 = 0xe0; P0 = ~menu1_table[i]; P2 = 0;
		}
    if(menu_index == 2) {
      P2 = 0xe0; P0 = ~menu2_table[i]; P2 = 0;
    }
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0;
		i++;
		if(i == 8) i = 0;
	}
}

#include "STC15F2K60S2.h"
#include "intrins.h"
#include "onewire.h"

typedef unsigned char u8;

u8 code smg_du[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x00};
u8 code smg_wei[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

bit key_flag;
bit pwm_flag = 0;
bit timer_restart_flag = 0;
bit timer_clear_flag = 0;
bit temp_read_flag = 0;
// mode
u8 global_mode;
u8 mode_zkb[4] = {0, 8, 7, 3}; // 占空比
// timer
u8 timer_mode;
u8 time_left;		// 当前时间
// smg
u8 smg_table[8];
// mode4
u8 temp_mode;

#define PWM_PORT P34
#define KEY P3
#define NO_KEY 0xff
#define KEY_STATE_0 0
#define KEY_STATE_1 1
#define KEY_STATE_2 2

u8 Key_Scan()
{
	static u8 key_state = KEY_STATE_0;
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

void Timer0Init(void)		//100微秒@11.0592MHz
{
	AUXR |= 0x80;		//定时器时钟1T模式
	TMOD &= 0xF0;		//设置定时器模式
	TL0 = 0xAE;		//设置定时初值
	TH0 = 0xFB;		//设置定时初值
	TF0 = 0;		//清除TF0标志
	TR0 = 1;		//定时器0开始计时
	ET0 = 1;
	EA = 1;
}

void timer_restart(void)
{
	time_left = timer_mode * 60;
	if(time_left == 0) {
		pwm_flag = 0;
		timer_clear_flag = 1;
	} else {
    timer_clear_flag = 0;
  }
	timer_restart_flag = 1;
	smg_table[4] = smg_du[time_left / 1000];
	smg_table[5] = smg_du[time_left / 100 % 10];
	smg_table[6] = smg_du[time_left / 10 % 10];
	smg_table[7] = smg_du[time_left % 10];
}

void smg_init(void)
{
	smg_table[0] = 0x40;
	smg_table[1] = smg_du[global_mode];
	smg_table[2] = 0x40;
	smg_table[3] = 0x00;
	smg_table[4] = smg_du[time_left / 1000];
	smg_table[5] = smg_du[time_left / 100 % 10];
	smg_table[6] = smg_du[time_left / 10 % 10];
	smg_table[7] = smg_du[time_left % 10];
}

void timer_mode_modify_f(void)
{
  if(time_left == 0) timer_mode = 0;
  else if(time_left <= 60) timer_mode = 1;
  else timer_mode = 2;
}

sbit buzzer = P0^6;
sbit relay = P0^4;

void main() 
{
  float temperature;
	u8 key_val = 0;
	P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0x00; // close buzzer & relay
	P2 = 0x80; P0 = ~0x00; P2 = 0; // close LED

	Timer0Init();
	global_mode = 1;	// 默认1模式启动
	timer_mode = 0;	// 定时器0分钟
	timer_restart();
	pwm_flag = 0;   // 关闭PWM输出
	smg_init();			// 数码管置位
	
	while(1) {
		if(timer_clear_flag == 1) { // 如果遇到停止指令、时间结束、定时器置0，标志位置1
			pwm_flag = 0;
		} else {
      pwm_flag = 1;
    }
    timer_mode_modify_f();
		if(global_mode != 4) {
			smg_table[4] = smg_du[time_left / 1000];
			smg_table[5] = smg_du[time_left / 100 % 10];
			smg_table[6] = smg_du[time_left / 10 % 10];
			smg_table[7] = smg_du[time_left % 10];
		} else {
      if(temp_read_flag) {
        temp_read_flag = 0;
//        EA = 0;
        temperature = read_temperature_f();
//        EA = 1;
        smg_table[5] = smg_du[(unsigned char)temperature / 10];
        smg_table[6] = smg_du[(unsigned char)temperature % 10];
//        smg_table[7] = smg_du[(unsigned char)(temperature * 10) % 10];
      }
    }
    
		if(key_flag) {
			key_flag = 0;
			key_val = Key_Scan();
			if(key_val) {
				switch(key_val) {
					case 4: 
						global_mode++;
						if(global_mode == 4) global_mode = 1;
					  smg_table[1] = smg_du[global_mode];
						break;
					case 5: 
						timer_mode++;
						if(timer_mode == 3) timer_mode = 0;
						timer_restart();
						break;
					case 6:
					  time_left = 0;
						timer_clear_flag = 1;
						break;
					case 7: 
            if(global_mode != 4) {
              temp_mode = global_mode;
              global_mode = 4;
              temp_read_flag = 1;
              smg_table[1] = smg_du[global_mode];
              smg_table[4] = 0x00;
              smg_table[7] = 0x39;
            } else {
              global_mode = temp_mode;
              smg_table[1] = smg_du[global_mode];
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

void Timer0(void) interrupt 1 using 1
{
	static unsigned int smg_cnt = 0, key_cnt = 0, i = 0;
	static unsigned int pwm_cnt = 0, timer_cnt = 0;
  static unsigned int temper_cnt = 0;
	smg_cnt++; key_cnt++;
	
	if(timer_restart_flag) {
		timer_restart_flag = 0;
		timer_cnt = 0;
	}
	if(time_left) timer_cnt++;
	else timer_clear_flag = 1;
	if(timer_cnt == 10000) { // 1s
		timer_cnt = 0;
		time_left--;
	}
	
  if(global_mode == 4) {
    temper_cnt++;
    if(temper_cnt == 1000) {
      temp_read_flag = 1;
      temper_cnt = 0;
    }
  }
  
  
	if(pwm_flag) {
		pwm_cnt++;
		if(pwm_cnt == mode_zkb[global_mode] * 1000) {
			PWM_PORT = 1;
      P2 = 0x80; P0 = ~0x01; P2 = 0x00;
		}
		if(pwm_cnt == 10 * 1000) {
			PWM_PORT = 0;
      P2 = 0x80; P0 = 0xff; P2 = 0x00;
			pwm_cnt = 0;
		}
	} else {
		pwm_cnt = 0; // clear cnt;
	}
	if(smg_cnt == 30) {
		smg_cnt = 0;
		P2 = 0xc0; P0 = 0x0; P2 = 0x00;
		P2 = 0xe0; P0 = ~smg_table[i]; P2 = 0;
		P2 = 0xc0; P0 = smg_wei[i]; P2 = 0;
		i++;
		if(i == 8) i = 0;
	}
	if(key_cnt == 100) {
		key_cnt = 0;
		key_flag = 1;
	}
}
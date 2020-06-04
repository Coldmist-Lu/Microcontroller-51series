/* 1-7-1 PCF8591 IIC协议控制CH1:光敏电阻，CH3:滑动变阻器 */

#include "reg52.h"
#include "iic.h"
#include "intrins.h"

typedef unsigned char BYTE;
typedef unsigned int WORD;


BYTE code T_display[]={0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
BYTE code T_COM[]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};      //位码

BYTE adc_table[3];
BYTE volt_table[3];
//-----------------------------------------------

/* define constants */
#define FOSC 11059200L

#define T1MS (65536-FOSC/12/1000)   //1ms timer calculation method in 12T mode

/* define SFR */
sbit TEST_LED = P1^0;               //work LED, flash once per second

/* define variables */
WORD count;                         //1000 times counter

//-----------------------------------------------

/* Timer0 interrupt routine */
void tm0_isr() interrupt 1 using 1
{
		static unsigned char i;
    TL0 = T1MS;                     //reload timer0 low byte
    TH0 = T1MS >> 8;                //reload timer0 high byte
    if (count-- == 0)               //1ms * 1000 -> 1s
    {
        count = 1;               //reset counter
//        P2 = 0xe0; P0 = ~T_display[adc_table[i]]; P2 = 0x00;
        P2 = 0xe0; P0 = ~volt_table[i]; P2 = 0x00;
				P2 = 0xc0; P0 = T_COM[i]; P2 = 0x00;
				i++;
				if(i == 3) i = 0;
    }
}

//-----------------------------------------------

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


/* main program */
sbit buzzer = P0^6;
sbit relay = P0^4;

void main()
{
		float Volt;
		BYTE adc_val;
		P2 = 0xa0; buzzer = 0; relay = 0; P2 = 0;
    TMOD = 0x01;                    //set timer0 as mode1 (16-bit)
    TL0 = T1MS;                     //initial timer0 low byte
    TH0 = T1MS >> 8;                //initial timer0 high byte
    TR0 = 1;                        //timer0 start running
    ET0 = 1;                        //enable timer0 interrupt
    EA = 1;                         //open global interrupt switch
    count = 0;                      //initial counter

		write_adc(0x01);  // ch3
    while (1) {
			EA = 0;
			adc_val = read_adc(0x01);
			EA = 1;
			Volt = adc_val / 255.0f * 5;
			adc_table[0] = adc_val / 100;
			adc_table[1] = adc_val / 10 % 10;
			adc_table[2] = adc_val % 10;
			
			volt_table[0] = T_display[(BYTE)Volt] | 0x80;
			volt_table[1] = T_display[(BYTE)(Volt*10)%10];
			volt_table[2] = T_display[(BYTE)(Volt*100)%10];
			
			Delay10ms();
			
		}
}


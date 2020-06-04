#ifndef _IIC_H
#define _IIC_H

#include "STC15F2K60S2.h"
#include "intrins.h"

#define somenop Delay5us()
#define SlaveAddrW 0xA0
#define SlaveAddrR 0xA1

//总线引脚定义
sbit SDA = P2^1;  /* 数据线 */
sbit SCL = P2^0;  /* 时钟线 */

//函数声明
void Delay5us();
void IIC_Start(void); 
void IIC_Stop(void);  
void IIC_Ack(unsigned char ackbit); 
void IIC_SendByte(unsigned char byt); 
bit IIC_WaitAck(void);  
unsigned char IIC_RecByte(void); 
void write_adc(unsigned char add);
unsigned char read_adc(unsigned char add);
void write_AT24C02(unsigned char add, unsigned char data1);
unsigned char read_AT24C02(unsigned char add);

#endif
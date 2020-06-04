#ifndef _IIC_H
#define _IIC_H

#include "STC15F2K60S2.h"
#include "intrins.h"

void IIC_Start(void); 
void IIC_Stop(void);  
bit IIC_WaitAck(void);  
void IIC_SendAck(bit ackbit); 
void IIC_SendByte(unsigned char byt); 
unsigned char IIC_RecByte(void); 
void write_adc(unsigned char add);
unsigned char read_adc(unsigned char add);
void write_AT24C02(unsigned char add, unsigned char data1);
unsigned char read_AT24C02(unsigned char add);
#endif
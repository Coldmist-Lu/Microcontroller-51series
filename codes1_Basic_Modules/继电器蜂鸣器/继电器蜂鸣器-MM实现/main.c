/* 实验1-1-2 利用存储器映射开关蜂鸣器和继电器 */
/* 须先把跳帽J13接到MM口
   2019.1.13 by Coldmist.Lu */

#include "reg52.h"
#include "absacc.h"

void main()
{
	XBYTE[0xA000] = 0x00; // close the buzzer & relay
	// XBYTE[0xA000] = 0x40; // open the buzzer
	// XBYTE[0xA000] = 0x10; // open the relay
	// XBYTE[0xA000] = 0x50; // open the buzzer & relay
	while(1) {
		
	}
}

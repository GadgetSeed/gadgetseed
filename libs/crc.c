/** @file
    @brief	CRC演算

    @date	2019.12.01
    @author	Takashi SHUDO
*/

#include "crc.h"

/*
  右送りCRC-16-IBM
*/
#define CRC16POLY 0xa001

unsigned short crc16(unsigned short crc, unsigned char const *data, int len)
{
	int i, j;
	unsigned char *dp = (unsigned char *)data;

	for(i=0; i<len; i++) {
		crc ^= *dp;
		for(j=0; j<8; j++) {
			if(crc & 1) {
				crc = (crc >> 1) ^ CRC16POLY;
			} else {
				crc >>= 1;
			}
		}
		dp ++;
	}

	return ~crc;
}

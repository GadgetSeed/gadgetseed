/** @file
    @brief	CRC演算

    @date	2019.12.01
    @author	Takashi SHUDO
*/

#ifndef CRC_H
#define CRC_H

unsigned short crc16(unsigned short crc, unsigned char const *data, int len);

#endif // CRC_H

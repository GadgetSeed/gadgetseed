/** @file
    @brief	FatFs用下位レイヤI/F

    @date	2009.01.11
    @author	Takashi SHUDO
*/

#include "ff.h"
#include "datetime.h"

/** 
    @brief	FAT 時間を取得する

    @return	FAT 時間
*/
DWORD get_fattime (void)
{
	return fattime();
}

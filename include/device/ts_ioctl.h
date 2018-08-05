/** @file
    @brief	タッチセンサドライバ ioctl 用マクロ定義

    推奨デバイス名 : "ts"

    @date	2017.12.17
    @author	Takashi SHUDO
*/

#ifndef TS_IOCTL_H
#define TS_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_TS		"ts"	///< 標準タッチセンサデバイス名

struct st_ts_calib_data {
	int left_pos;
	int left_val;
	int right_pos;
	int right_val;

	int top_pos;
	int top_val;
	int bottom_pos;
	int bottom_val;
};	///< タッチセンサキャリブレーションデータ

#define IOCMD_TS_SET_CALIB	STDIOCTL(DEV_TS,0x00)	///< キャリブレーションデータを設定する

#endif // TS_IOCTL_H

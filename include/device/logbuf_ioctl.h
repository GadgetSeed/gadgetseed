/** @file
    @brief	ログバッファドライバ ioctl 用マクロ定義

    推奨デバイス名 : "logbuf"

    @date	2018.11.18
    @author	Takashi SHUDO
*/

#ifndef LOGBUF_IOCTL_H
#define LOGBUF_IOCTL_H

#include "device/std_ioctl.h"

struct st_loginfo {
	unsigned int maxsize;
	unsigned int logsize;
	unsigned int record_count;
	unsigned long long start_time;
	unsigned long long last_time;
};	///< ログ情報

#define DEF_DEV_NAME_LOGBUF	"logbuf"	///< ログバッファデバイス名(バッファのみ)
#define DEF_DEV_NAME_LOGOUT	"logout"	///< ログ出力デバイス名(出力とバッファ)

#define IOCMD_LOGBUF_GET_INFO		STDIOCTL(DEV_LOGBUF,0x01)	///< ログ情報を取得
#define IOCMD_LOGBUF_SEEK_HEAD		STDIOCTL(DEV_LOGBUF,0x02)	///< ログを先頭よりシークする
#define IOCMD_LOGBUF_SEEK_TAIL		STDIOCTL(DEV_LOGBUF,0x03)	///< ログを終端よりシークする
#define IOCMD_LOGBUF_SEEK_TIME		STDIOCTL(DEV_LOGBUF,0x04)	///< ログを終端よりシークする
#define IOCMD_LOGBUF_GET_LINESIZE	STDIOCTL(DEV_LOGBUF,0x05)	///< シークポイントの1ログの長さを取得する

#endif // LOGBUF_IOCTL_H

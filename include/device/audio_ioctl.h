/** @file
    @brief	Audioドライバ ioctl 用マクロ定義

    推奨デバイス名 : "audio"

    @date	2017.02.11
    @date	2012.01.11
    @author	Takashi SHUDO
*/

#ifndef AUDIO_IOCTL_H
#define AUDIO_IOCTL_H

#include "device/std_ioctl.h"

#define DEF_DEV_NAME_AUDIO	"audio"	///< 標準AUDIOデバイス名

// ボリューム設定
#define IOCMD_AUDIO_SET_VOLUME		STDIOCTL(DEV_AUDIO,0x00)	///< ボリュームを設定する
#define IOCMD_AUDIO_GET_VOLUME		STDIOCTL(DEV_AUDIO,0x01)	///< ボリュームを取得する
#define IOCMD_AUDIO_SET_MUTE		STDIOCTL(DEV_AUDIO,0x02)	///< ミュートを設定、解除する

// バッファ設定
#define IOCMD_AUDIO_SET_BUFSIZE		STDIOCTL(DEV_AUDIO,0x10)	///< オーディオバッファサイズを設定する
#define IOCMD_AUDIO_GET_BUFSIZE		STDIOCTL(DEV_AUDIO,0x11)	///< オーディオバッファサイズを取得する

// 再生、停止
#define IOCMD_AUDIO_PLAY_START		STDIOCTL(DEV_AUDIO,0x20)	///< オーディオ再生を開始する
#define IOCMD_AUDIO_PLAY_STOP		STDIOCTL(DEV_AUDIO,0x21)	///< オーディオ再生を停止
#define IOCMD_AUDIO_GET_STATUS		STDIOCTL(DEV_AUDIO,0x30)	///< オーディオ再生状態を取得する

// サンプリングレート設定
#define IOCMD_AUDIO_SET_SMPRATE		STDIOCTL(DEV_AUDIO,0x40)	///< サンプリングレートを設定する
#define IOCMD_AUDIO_GET_SMPRATE		STDIOCTL(DEV_AUDIO,0x41)	///< サンプリングレートを取得する

// バッファ取得
#define IOCMD_AUDIO_GET_BUFFER		STDIOCTL(DEV_AUDIO,0x50)	///< オーディオバッファメモリアドレスを取得する
#define IOCMD_AUDIO_WAIT_BUFFER		STDIOCTL(DEV_AUDIO,0x51)	///< オーディオバッファが空くのを待つ

#endif // AUDIO_IOCTL_H

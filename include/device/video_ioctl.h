/** @file
    @brief	画像表示デバイスドライバ ioctl 用マクロ定義

    推奨デバイス名 : "video"

    @date	2017.01.15
    @author	Takashi SHUDO
*/

#ifndef VIDEO_IOCTL_H
#define VIDEO_IOCTL_H

#include "device/std_ioctl.h"
#include "framebuf.h"

#define DEF_DEV_NAME_VIDEO	"video"	///< 標準ビデオデバイス名

// ビデオデバイスタイプ(video_info.type)
#define VIDEOTYPE_MONO		0	///< モノクロ(2値)フレームバッファ
#define VIDEOTYPE_CMDDRAW	1	///< コマンドによる描画(フレームバッファなし)
#define VIDEOTYPE_FRAMEBUF	2	///< カラーフレームバッファ(描画同期無し)

// 色深度(video_info.color_depth)
#define VCOLORDEP_MONO	0	///< モノクロ
#define VCOLORDEP_8	1	///< 8ビットカラー
#define VCOLORDEP_16	2	///< 16ビットカラー
#define VCOLORDEP_24	3	///< 24ビットカラー
#define VCOLORDEP_32	4	///< 32ビットカラー

struct st_video_info {
	unsigned short type;	///< ビデオデバイスタイプ(VIDEOTYPE_*)
	unsigned short width;	///< 表示幅ドット数
	unsigned short height;	///< 表示高さドット数
	unsigned short color_depth;	///< 色深度(VCOLORDEP_*)
	unsigned short frame_num;	///< 表示可能フレーム数
	unsigned char *frame_buf_ptr[MAX_FRAMEBUF];	///< フレームバッファポインタ
	unsigned int mem_size;	///< フレームバッファメモリサイズ
	struct st_device *dev;	///< ビデオデバイスドライバ
};	///< 画像表示デバイス情報


// フレームバッファ制御 ioctl

#define IOCMD_VIDEO_LOCK_DEVICE		STDIOCTL(DEV_VIDEO,0x00)	///< デバイスをMUTEXロックする
#define IOCMD_VIDEO_UNLOCK_DEVICE	STDIOCTL(DEV_VIDEO,0x01)	///< デバイスをMUTEXアンロックする

#define IOCMD_VIDEO_SETDISPFRAME	STDIOCTL(DEV_VIDEO,0x02)	///< 表示フレーム番号を設定する
#define IOCMD_VIDEO_GETDISPFRAME	STDIOCTL(DEV_VIDEO,0x03)	///< 表示フレーム番号を取得する
#define IOCMD_VIDEO_SETDRAWFRAME	STDIOCTL(DEV_VIDEO,0x04)	///< 描画フレーム番号を設定する
#define IOCMD_VIDEO_GETDRAWFRAME	STDIOCTL(DEV_VIDEO,0x05)	///< 描画フレーム番号を取得する

#define IOCMD_VIDEO_CLEAR		STDIOCTL(DEV_VIDEO,0x20)	///< 全画面初期化
#define IOCMD_VIDEO_SCROLL		STDIOCTL(DEV_VIDEO,0x22)	///< 表示位置を変更(スクロール)する
#define IOCMD_VIDEO_FILL		STDIOCTL(DEV_VIDEO,0x24)	///< 全画面を任意の色に描画する
#define IOCMD_VIDEO_SETRECT		STDIOCTL(DEV_VIDEO,0x25)	///< 描画データ転送範囲を矩形で設定する
#define IOCMD_VIDEO_RESETRECT		STDIOCTL(DEV_VIDEO,0x26)	///< 描画データ転送範囲を全表示範囲にする

#define IOCMD_VIDEO_WRITE_BYTE		STDIOCTL(DEV_VIDEO,0x40)	///< 1バイト表示データを転送する(未使用)
#define IOCMD_VIDEO_WRITE_WORD		STDIOCTL(DEV_VIDEO,0x41)	///< 2バイト表示データを転送する(未使用)
#define IOCMD_VIDEO_WRITE_LONG		STDIOCTL(DEV_VIDEO,0x42)	///< 4バイト表示データを転送する(未使用)

#define IOCMD_VIDEO_NOLOCK_WRITE_WORD	STDIOCTL(DEV_VIDEO,0x44)	///< 2バイト表示データを転送する(MUTEXロックは無視)

#define IOCMD_VIDEO_SET_FORECOLOR	STDIOCTL(DEV_VIDEO,0x60)	///< フォアカラーを設定する
#define IOCMD_VIDEO_SET_BACKCOLOR	STDIOCTL(DEV_VIDEO,0x61)	///< バックカラーを設定する
#define IOCMD_VIDEO_REPEAT_DATA		STDIOCTL(DEV_VIDEO,0x62)	///< 指定ドット数分フォアカラーで描画する
#define IOCMD_VIDEO_DRAW_PIXEL		STDIOCTL(DEV_VIDEO,0x63)	///< フォアカラーで1ドット描画する
#define IOCMD_VIDEO_DRAW_BITS		STDIOCTL(DEV_VIDEO,0x64)	///< ビットパターンを描画する

#define IOCMD_VIDEO_BCKLIGHT		STDIOCTL(DEV_VIDEO,0x70)	///< バックライトを制御する(未使用)

#endif // VIDEO_IOCTL_H

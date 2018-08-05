/** @file
    @brief	フレームバッファドライバ用コンテキスト定義

    @date	2007.03.17
    @author	Takashi SHUDO
*/

#ifndef FRAMEBUF_H
#define FRAMEBUF_H

#include "sysconfig.h"
#include "device.h"
#include "graphics.h"

struct st_framebuf_context {
	struct st_device *v_dev;		///< 下位デバイスドライバ
	short width;				///< フレームバッファ幅ピクセル数
	short height;				///< フレームバッファ高さピクセル数
	unsigned short pixcel_byte;		///< 1ピクセルのデータバイト数
	unsigned short disp_frame;		///< 表示フレームバッファ番号
	unsigned short draw_frame;		///< 描画フレームバッファ番号
	unsigned char *fb_ptr[MAX_FRAMEBUF];	///< フレームバッファメモリポインタ
	unsigned int mem_size;			///< 1フレームのメモリバイト数

	unsigned int fore_color;		///< フォアカラー
	unsigned int back_color;		///< バックカラー
	struct st_rect clip;			///< 描画クリッピングエリア
	short pen_x;				///< 描画ペンのX座標
	short pen_y;				///< 描画ペンのY座標
	unsigned char *draw_ptr;		///< 描画ペンのメモリアドレス
};	///< フレームバッファコンテキスト

#endif // FRAMEBUF_H

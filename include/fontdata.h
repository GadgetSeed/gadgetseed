/** @file
    @brief	フォントデータ構造

    @date	2007.06.26
    @author	Takashi Shudo
*/

#ifndef FONTDATA_H
#define FONTDATA_H

#define MAX_FONTNAMELEN	32	///< 最大フォント名長

struct st_font {
	unsigned short start;	///< 開始文字コード
	unsigned short end;	///< 終了文字コード
	unsigned short width;	///< 1文字の幅(ピクセル数)
	unsigned short height;	///< 1文字の高さ(ピクセル数)
	unsigned short dwidth;	///< 1文字のビットマップデータのバイト数
	unsigned int *index;	///< 文字コードとビットマップデータのバイト位置の対応データ
	signed char *bitmap;	///< 各文字フォントのビットマップデータ
};	///< フォントデータ

struct st_fontset {
	char name[MAX_FONTNAMELEN];	///< フォントセット名
	struct st_font *font;		///< 半角フォントデータポインタ
	struct st_font *w_font;		///< 全角フォントデータポインタ
};	///< フォントセット

#endif // FONTDATA_H

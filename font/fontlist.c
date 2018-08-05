/** @file
    @brief	フォント

    @date	2007.06.26
    @author	Takashi Shudo

    @page font_list フォントリスト

    GadgetSeedは複数の文字フォントによる描画が可能です。

    GadgetSeedは以下のフォントが使用できます。

    | フォント名	| コンフィグ名				| 詳細					| URL	|
    |:------------------|:--------------------------------------|:--------------------------------------|-------|
    | 4x6		| FONTS_ENABLE_FONT_4X6			| オリジナル4x6サイズ半角フォント	|	|
    | 4x8		| FONTS_ENABLE_FONT_4X8			| オリジナル4x8サイズ半角フォント	|	|
    | 6x6		| FONTS_ENABLE_FONT_6X6			| オリジナル6x6サイズ半角フォント	|	|
    | 8x16		| FONTS_ENABLE_FONT_8X16		| オリジナル8x16サイズ半角フォント	|	|
    | 12x16		| FONTS_ENABLE_FONT_12X16		| オリジナル12x16サイズ半角フォント	|	|
    | 12x24		| FONTS_ENABLE_FONT_12X24		| オリジナル12x24サイズ半角フォント	|	|
    | num24x32		| FONTS_ENABLE_FONT_NUM24X32		| オリジナル24x32サイズ数字フォント	|	|
    | num24x40		| FONTS_ENABLE_FONT_NUM24X40		| オリジナル24x40サイズ数字フォント	|	|
    | num24x48		| FONTS_ENABLE_FONT_NUM24X48		| オリジナル24x48サイズ数字フォント	|	|
    | num48x64		| FONTS_ENABLE_FONT_NUM48X64		| オリジナル48x64サイズ数字フォント	|	|
    | misaki		| FONTS_ENABLE_FONT_MISAKI		| 美咲フォント 8x8サイズ漢字フォント	| http://www.geocities.jp/littlimi/misaki.htm	|
    | knj10		| FONTS_ENABLE_FONT_NAGA10		| ナガ10フォント 10x10サイズ漢字フォント	| https://github.com/r-lyeh-archived/fortfont/tree/master/kochi-mincho/docs/naga10	|
    | jiskan16		| FONTS_ENABLE_FONT_JISKAN16		| jiskan16フォント 16x16サイズ漢字フォント	|	|
    | jiskan24		| FONTS_ENABLE_FONT_JISKAN24		| jiskan24フォント 24x24サイズ漢字フォント	|	|
    | genshingothic_p18	| FONTS_ENABLE_FONT_GENSHINGOTHIC	| 源真ゴシック 18ポイント漢字フォント	| http://jikasei.me/font/genshin/	|
    | mplus_p2_p18	| FONTS_ENABLE_FONT_MPLUS		| M+ 18ポイント漢字フォント		| https://mplus-fonts.osdn.jp/	|

    misaki、knj10、jiskan16、jiskan24 はオリジナルの BDF フォントを GadgetSeed 独自の漢字フォント形式に変換し、表示します。

    genshingothic_p18、mplus_p2_p18 はオリジナルの TrueType フォントを GadgetSeed 独自の漢字フォント形式に変換し、表示します。
    これは、実験的な試みです。
*/

#include "sysconfig.h"

#include "font.h"

extern const struct st_fontset * const fontptr_4x6;
extern const struct st_fontset * const fontptr_4x8;
extern const struct st_fontset * const fontptr_6x6;
extern const struct st_fontset * const fontptr_8x16;
extern const struct st_fontset * const fontptr_12x16;
extern const struct st_fontset * const fontptr_12x24;
extern const struct st_fontset * const fontptr_16x24;
extern const struct st_fontset * const fontptr_num24x32;
extern const struct st_fontset * const fontptr_num24x40;
extern const struct st_fontset * const fontptr_num24x48;
extern const struct st_fontset * const fontptr_num48x64;
extern const struct st_fontset * const fontptr_misaki;
extern const struct st_fontset * const fontptr_knj10;
extern const struct st_fontset * const fontptr_jiskan16;
extern const struct st_fontset * const fontptr_jiskan24;
extern const struct st_fontset * const fontptr_genshingothic_p18;
extern const struct st_fontset * const fontptr_mplus_p2_p18;

const struct st_fontset * const * const font_list[] = {
//#ifdef GSC_FONTS_ENABLE_FONT_8X16	// $gsc 8X16フォントを有効にする
	&fontptr_8x16,
//#endif

#ifdef GSC_FONTS_ENABLE_FONT_4X6	// $gsc 4X6フォントを有効にする
	&fontptr_4x6,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_4X8	// $gsc 4X8フォントを有効にする
	&fontptr_4x8,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_6X6	// $gsc 6X6フォントを有効にする
	&fontptr_6x6,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_12X16	// $gsc 12X16フォントを有効にする
	&fontptr_12x16,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_12X24	// $gsc 12X24フォントを有効にする
	&fontptr_12x24,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_16X24	// $gsc 16X24フォントを有効にする
	&fontptr_16x24,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_NUM24X32	// $gsc NUM24X32フォントを有効にする
	&fontptr_num24x32,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_NUM24X40	// $gsc NUM24X40フォントを有効にする
	&fontptr_num24x40,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_NUM24X48	// $gsc NUM24X48フォントを有効にする
	&fontptr_num24x48,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_NUM48X64	// $gsc NUM48X64フォントを有効にする
	&fontptr_num48x64,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_MISAKI	// $gsc 美咲フォントを有効にする
	&fontptr_misaki,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_NAGA10	// $gsc ナガ10フォントを有効にする
	&fontptr_knj10,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_JISKAN16	// $gsc jiskan16フォントを有効にする
	&fontptr_jiskan16,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_JISKAN24	// $gsc jiskan24フォントを有効にする
	&fontptr_jiskan24,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_GENSHINGOTHIC	// $gsc 源真ゴシックフォントを有効にする
	&fontptr_genshingothic_p18,
#endif

#ifdef GSC_FONTS_ENABLE_FONT_MPLUS	// $gsc M+フォントを有効にする
	&fontptr_mplus_p2_p18,
#endif
	0
};

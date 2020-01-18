/** @file
    @brief	UIスタイル

    @date	2019.08.10
    @author	Takashi SHUDO
*/

#ifndef UI_STYLE_H
#define UI_STYLE_H

#include "sysconfig.h"
#include "graphics.h"

#ifdef GSC_UISTYLEDEF
#include GSC_UISTYLEDEF	// $gsc UIスタイル定義ヘッダファイル名
#endif

#ifndef GSC_FONTS_DEFAULT_FONT	/// $gsc デフォルトのフォント名
#define GSC_FONTS_DEFAULT_FONT "8x16"
#endif

//#define UISTYLE_ORANGE
//#define UISTYLE_GREEN
//#define UISTYLE_BLUE

#ifdef UISTYLE_ORANGE
#define UI_BACK_COLOR		RGB(0,0,0)		// 背景色
#define UI_NORMAL_FORE_COLOR	RGB(220,120,120)	// UI通常前面色
#define UI_NORMAL_BACK_COLOR	RGB(100,30,30)		// UI通常背景色
#define UI_ACTIVE_FORE_COLOR	RGB(250,150,150)	// UIアクティブなオブジェクトの前面色
#define UI_ACTIVE_BACK_COLOR	RGB(200,50,50)		// UIアクティブなオブジェクトの背景色
#define UI_INACTIVE_FORE_COLOR	RGB(180,80,80)		// UIインアクティブなオブジェクトの前面色
#define UI_INACTIVE_BACK_COLOR	RGB(150,50,50)		// UIインアクティブなオブジェクトの背景色
#define UI_CURSOR_FORE_COLOR	RGB(210,150,150)	// UIアクティブなテキストカーソルの前面色
#define UI_CURSOR_BACK_COLOR	RGB(50,20,20)		// UIアクティブなテキストカーソルの背景色
#define UI_ON_COLOR		RGB(250,50,50)		// UI On色
#define UI_OFF_COLOR		RGB(100,50,50)		// UI Off色
#endif

#ifdef UISTYLE_GREEN
#define UI_BACK_COLOR		RGB(0,0,0)		// 背景色
#define UI_NORMAL_FORE_COLOR	RGB(120,220,120)	// UI通常前面色
#define UI_NORMAL_BACK_COLOR	RGB(20,60,20)		// UI通常背景色
#define UI_ACTIVE_FORE_COLOR	RGB(150,250,150)	// UIアクティブなオブジェクトの前面色
#define UI_ACTIVE_BACK_COLOR	RGB(50,200,50)		// UIアクティブなオブジェクトの背景色
#define UI_INACTIVE_FORE_COLOR	RGB(80,180,80)		// UIインアクティブなオブジェクトの前面色
#define UI_INACTIVE_BACK_COLOR	RGB(50,150,50)		// UIインアクティブなオブジェクトの背景色
#define UI_CURSOR_FORE_COLOR	RGB(150,210,150)	// UIアクティブなテキストカーソルの前面色
#define UI_CURSOR_BACK_COLOR	RGB(20,50,20)		// UIアクティブなテキストカーソルの背景色
#define UI_ON_COLOR		RGB(50,250,50)		// UI On色
#define UI_OFF_COLOR		RGB(50,150,50)		// UI Off色
#endif

#ifdef UISTYLE_BLUE
#define UI_BACK_COLOR		RGB(20,20,60)		// 背景色
#define UI_NORMAL_FORE_COLOR	RGB(120,120,220)	// UI通常前面色
#define UI_NORMAL_BACK_COLOR	RGB(20,20,60)		// UI通常背景色
#define UI_ACTIVE_FORE_COLOR	RGB(150,150,250)	// UIアクティブなオブジェクトの前面色
#define UI_ACTIVE_BACK_COLOR	RGB(50,50,200)		// UIアクティブなオブジェクトの背景色
#define UI_INACTIVE_FORE_COLOR	RGB(80,80,180)		// UIインアクティブなオブジェクトの前面色
#define UI_INACTIVE_BACK_COLOR	RGB(50,50,150)		// UIインアクティブなオブジェクトの背景色
#define UI_CURSOR_FORE_COLOR	RGB(20,20,50)		// UIアクティブなテキストカーソルの前面色
#define UI_CURSOR_BACK_COLOR	RGB(150,150,210)	// UIアクティブなテキストカーソルの背景色
#define UI_ON_COLOR		RGB(50,50,250)		// UI On色
#define UI_OFF_COLOR		RGB(50,50,150)		// UI Off色
#endif

#ifndef UI_BACK_COLOR
#define UI_BACK_COLOR		RGB(0,0,0)		// 背景色
#endif

#ifndef UI_NORMAL_FORE_COLOR
#define UI_NORMAL_FORE_COLOR	RGB(220,220,220)	// UI前面色
#endif

#ifndef UI_NORMAL_BACK_COLOR
#define UI_NORMAL_BACK_COLOR	RGB(50,50,50)		// UI前面色
#endif

#ifndef UI_ACTIVE_FORE_COLOR
#define UI_ACTIVE_FORE_COLOR	RGB(250,250,250)	// UIアクティブなオブジェクトの前面色
#endif

#ifndef UI_ACTIVE_BACK_COLOR
#define UI_ACTIVE_BACK_COLOR	RGB(150,150,150)	// UIアクティブなオブジェクトの背景色
#endif

#ifndef UI_INACTIVE_FORE_COLOR
#define UI_INACTIVE_FORE_COLOR	RGB(80,80,80)		// UIインアクティブなオブジェクトの前面色
#endif

#ifndef UI_INACTIVE_BACK_COLOR
#define UI_INACTIVE_BACK_COLOR	RGB(80,80,80)		// UIインアクティブなオブジェクトの背景色
#endif

#ifndef UI_CURSOR_BACK_COLOR
#define UI_CURSOR_BACK_COLOR	RGB(20,20,20)		// UIアクティブなテキストカーソルの背景色
#endif

#ifndef UI_CURSOR_FORE_COLOR
#define UI_CURSOR_FORE_COLOR	RGB(210,250,250)	// UIアクティブなテキストカーソルの前面色
#endif

#ifndef UI_ON_COLOR
#define UI_ON_COLOR		RGB(50,200,50)		// UI On色
#endif

#ifndef UI_OFF_COLOR
#define UI_OFF_COLOR		RGB(50,50,50)		// UI Off色
#endif

#endif // UI_STYLE_H

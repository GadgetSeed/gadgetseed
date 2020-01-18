/** @file
    @brief	グラフィックス

    図形演算ライブラリ

    @date	2013.06.20
    @date	2007.03.20

    @author	Takashi SHUDO
*/

#include "graphics.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

/**
   @brief	２つの矩形のandを求める

   @param[out]	a	２つの矩形のand矩形
   @param[in]	s1	矩形1
   @param[in]	s2	矩形2
*/
void and_rect(struct st_rect *a, struct st_rect *s1, struct st_rect *s2)
{
	/* 範囲外？ */
	if((s1->top > s2->bottom)
	   || (s1->bottom < s2->top)
	   || (s1->left > s2->right)
	   || (s1->right < s2->left)) {
		a->left		= 0;
		a->top		= 0;
		a->right	= 0;
		a->bottom	= 0;
		return;
	}

	if(s1->left > s2->left) {
		a->left = s1->left;
	} else {
		a->left = s2->left;
	}

	if(s1->top > s2->top) {
		a->top = s1->top;
	} else {
		a->top = s2->top;
	}

	if(s1->right > s2->right) {
		a->right = s2->right;
	} else {
		a->right = s1->right;
	}

	if(s1->bottom > s2->bottom) {
		a->bottom = s2->bottom;
	} else {
		a->bottom = s1->bottom;
	}
}

/**
   @brief	２つの矩形のorを求める

   @param[out]	a	２つの矩形のor矩形
   @param[in]	s1	矩形1
   @param[in]	s2	矩形2
*/
void or_rect(struct st_rect *a, struct st_rect *s1, struct st_rect *s2)
{
	if(s1->left > s2->left) {
		a->left = s2->left;
	} else {
		a->left = s1->left;
	}

	if(s1->top > s2->top) {
		a->top = s2->top;
	} else {
		a->top = s1->top;
	}

	if(s1->right > s2->right) {
		a->right = s1->right;
	} else {
		a->right = s2->right;
	}

	if(s1->bottom > s2->bottom) {
		a->bottom = s1->bottom;
	} else {
		a->bottom = s2->bottom;
	}
}

/**
   @brief	矩形が0か調べる

   @param	rect	矩形

   @return	=1:矩形は0
*/
short empty_rect(struct st_rect *rect)
{
	if(rect->left == rect->right) {
		return 1;
	}

	if(rect->top == rect->bottom) {
		return 1;
	}

	return 0;
}

/**
   @brief	長方形が0か調べる

   @param	box	長方形

   @return	=1:長方形は0
*/
short empty_box(struct st_box *box)
{
	if(box->sur.width == 0) {
		return 1;
	}

	if(box->sur.height == 0) {
		return 1;
	}

	return 0;
}

/**
   @brief	矩形を修正する

   @param[in,out]	rect	矩形
*/
void correct_rect(struct st_rect *rect)
{
	short tmp;

	if(rect->left > rect->right) {
		tmp = rect->left;
		rect->left = rect->right;
		rect->right = tmp;
	}

	if(rect->top > rect->bottom) {
		tmp = rect->top;
		rect->top = rect->bottom;
		rect->bottom = tmp;
	}
}

/**
   @brief	座標が矩形の内部か調べる

   @param[in]	x	座標X
   @param[in]	y	座標Y
   @param[in]	rect	矩形

   @return	=1:座標は矩形の中
*/
int is_point_in_rect(short x, short y, struct st_rect *rect)
{
	if(x < rect->left) {
		return 0;
	}

	if(rect->right < x) {
		return 0;
	}

	if(y < rect->top) {
		return 0;
	}

	if(rect->bottom < y) {
		return 0;
	}

	return 1;
}

/**
   @brief	座標が四角の内部か調べる

   @param[in]	x	座標X
   @param[in]	y	座標Y
   @param[in]	box	四角

   @return	=1:座標は四角の中
*/
int is_point_in_box(short x, short y, struct st_box *box)
{
	struct st_rect rect;

	rect.left = box->pos.x;
	rect.top = box->pos.y;
	rect.right = box->pos.x + box->sur.width;
	rect.bottom = box->pos.y + box->sur.height;

	return is_point_in_rect(x, y, &rect);
}

/**
   @brief	box -> rect 変換

   @param[out]	rect	矩形
   @param[in]	box	四角
*/
void box2rect(struct st_rect *rect, struct st_box *box)
{
	rect->left = box->pos.x;
	rect->top = box->pos.y;
	rect->right = box->pos.x + box->sur.width;
	rect->bottom = box->pos.y + box->sur.height;
}

/**
   @brief	イメージデータのサイズを変更する

   @param[out]	dst_image	リサイズ後イメージデータ
   @param[in]	dwidth		リサイズ後イメージデータ幅
   @param[in]	dwidth		リサイズ後イメージデータ高さ
   @param[in]	srctimage	リサイズ前イメージデータ
   @param[in]	dwidth		リサイズ前イメージデータ幅
   @param[in]	dwidth		リサイズ前イメージデータ高さ
*/
void resize_image(void *dst_image, short dwidth, short dheight, 
		  void *src_image, short swidth, short sheight)
{
	int i, j;

	PIXEL_DATA *dst = (PIXEL_DATA *)dst_image;
	PIXEL_DATA *src = (PIXEL_DATA *)src_image;

	// 縮小時
	if((dwidth <= swidth) && (dheight <= sheight)) {
		for(j=0; j<dheight; j++) {
			int sy = (sheight * j)/dheight;
			for(i=0; i<dwidth; i++) {
				int sx = (swidth * i)/dwidth;
				DTPRINTF(0x02, "(%d, %d) < (%d, %d)\n", i, j, sx, sy);
				dst[(dwidth * j) + i] = src[(swidth * sy) + sx];
			}
		}
	}

	// 拡大時(未実装)
}

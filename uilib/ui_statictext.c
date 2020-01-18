/** @file
    @brief	ユーザインタフェース - 静的テキスト

    @date	2019.11.17
    @auther	Takashi SHUDO
*/

#include "font.h"
#include "tkprintf.h"
#include "ui_style.h"
#include "ui_statictext.h"

//#define DEBUGVIEW

//#define DEBUGTBITS 0x01
#include "dtprintf.h"

void draw_ui_statictext(struct st_ui_statictext *statictext)
{
	struct st_ui_statictext *st = statictext;

	if(st->fore_color != 0) {
		set_forecolor(*(st->fore_color));
	}

	if(st->back_color != 0) {
		set_backcolor(*(st->back_color));
	}

	set_draw_mode(GRP_DRAWMODE_NORMAL);

	if(st->font_name != 0) {
		(void)set_font_by_name(st->font_name);
	} else {
		(void)set_font_by_name(GSC_FONTS_DEFAULT_FONT);
	}

	if(st->fillattr == UI_STATICTEXT_FILLARRT_FILL) {
		struct st_box tbox = { 0 }, bbox = { 0 };
		struct st_box lbox = { 0 }, rbox = { 0 };
		int str_w = 0, str_h = 0;

		str_w = str_width(st->text);
		str_h = font_height();
		DTPRINTF(0x01, "str_w = %d, str_h = %d\n", str_w, str_h);

		tbox.pos.x = st->view_area.pos.x;
		tbox.sur.width = st->view_area.sur.width;
		lbox.sur.height = str_h;
		rbox.sur.height = str_h;
		bbox.pos.x = st->view_area.pos.x;
		bbox.sur.width = st->view_area.sur.width;

		switch(st->hattr) {
		case FONT_HATTR_LEFT:
			rbox.pos.x = st->view_area.pos.x + str_w;
			rbox.sur.width = st->view_area.sur.width - str_w;
			if(rbox.sur.width < 0) {
				rbox.sur.width = 0;
			}
			break;

		case FONT_HATTR_CENTER:
			lbox.pos.x = st->view_area.pos.x;
			lbox.sur.width = (st->view_area.sur.width - str_w)/2;
			rbox.pos.x = st->view_area.pos.x + str_w + (st->view_area.sur.width - str_w)/2;
			rbox.sur.width = (st->view_area.sur.width - str_w)/2;
			if(rbox.sur.width < 0) {
				rbox.sur.width = 0;
			}
			break;

		case FONT_HATTR_RIGHT:
			lbox.pos.x = st->view_area.pos.x;
			lbox.sur.width = st->view_area.sur.width - str_w;
			break;

		default:
			SYSERR_PRINT("Unknow hattr %d\n", st->hattr);
			break;
		}

		switch(st->vattr) {
		case FONT_VATTR_TOP:
			lbox.pos.y = st->view_area.pos.y;
			rbox.pos.y = st->view_area.pos.y;
			bbox.pos.y = st->view_area.pos.y + str_h;
			bbox.sur.height = st->view_area.sur.height - str_h;
			break;

		case FONT_VATTR_CENTER:
			tbox.pos.y = st->view_area.pos.y;
			tbox.sur.height = (st->view_area.sur.height - str_h)/2;
			lbox.pos.y = st->view_area.pos.y + (st->view_area.sur.height - str_h)/2;
			rbox.pos.y = st->view_area.pos.y + (st->view_area.sur.height - str_h)/2;
			bbox.pos.y = st->view_area.pos.y + str_h + (st->view_area.sur.height - str_h)/2;
			bbox.sur.height = (st->view_area.sur.height - str_h)/2;
			break;

		case FONT_VATTR_BOTTOM:
			tbox.pos.y = st->view_area.pos.y;
			tbox.sur.height = st->view_area.sur.height - str_h;
			lbox.pos.y = st->view_area.pos.y + (str_h/2);
			rbox.pos.y = st->view_area.pos.y + (str_h/2);
			break;

		default:
			SYSERR_PRINT("Unknow vattr %d\n", st->hattr);
			break;
		}

		set_draw_mode(GRP_DRAWMODE_REVERSE);
		if(empty_box(&tbox) == 0) {
			draw_fill_box(&tbox);
		}
		if(empty_box(&lbox) == 0) {
			draw_fill_box(&lbox);
		}
		set_draw_mode(GRP_DRAWMODE_NORMAL);
		//set_clip_box(&(st->view_area));
		draw_str_in_box((struct st_box *)&(st->view_area), st->hattr, st->vattr, st->text);
		//clear_clip_rect();
		set_draw_mode(GRP_DRAWMODE_REVERSE);
		if(empty_box(&rbox) == 0) {
			draw_fill_box(&rbox);
		}
		if(empty_box(&bbox) == 0) {
			draw_fill_box(&bbox);
		}
		set_draw_mode(GRP_DRAWMODE_NORMAL);
	} else {
		// 文字がview_areaをはみ出してはいけない
		// [TODO]文字がview_areaをはみ出す場合クリッピングする
		//set_clip_box(&(st->view_area));
		draw_str_in_box((struct st_box *)&(st->view_area), st->hattr, st->vattr, st->text);
		//clear_clip_rect();
	}

#ifdef DEBUGVIEW
	set_forecolor(RGB(255,0,0));
	draw_box(&(st->view_area));
#endif
}

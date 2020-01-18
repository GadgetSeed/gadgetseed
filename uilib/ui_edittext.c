/** @file
    @brief	ユーザインタフェース - テキストエディット

    @date	2018.01.29
    @auther	Takashi SHUDO
*/

#include "font.h"
#include "ui_style.h"
#include "ui_edittext.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

static const struct st_graph_object te_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object te_uneditable_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_INACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object te_active_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_ACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object te_cursor_view[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_CURSOR_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ UI_CURSOR_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static void draw_cursor(struct st_ui_edittext *te, int flg_on)
{
	short x, y;
	short w;
	int i;

	if(te->font_name != 0) {
		(void)set_font_by_name(te->font_name);
	} else {
		(void)set_font_by_name(GSC_FONTS_DEFAULT_FONT);
	}
	w = font_width(' ');

	y = te->view_area.pos.y + te->text_area_margin;

	if(flg_on != 0) {
		if(te->cursor_view == 0) {
			draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te_cursor_view);
		} else {
			draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->cursor_view);
		}
	} else {
		if(te->active_view == 0) {
			draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te_active_view);
		} else {
			draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->active_view);
		}
	}

	DTPRINTF(0x01, "TOP=%d, END=%d\n", te->cursor_pos_top, te->cursor_pos_end);
	if(te->cursor_pos_end > te->cursor_pos_top) {
		x = te->view_area.pos.x + te->text_area_margin + (w * te->cursor_pos_top);
		for(i=te->cursor_pos_top; i<te->cursor_pos_end+1; i++) {
			if(te->text[i] == 0) {
				draw_char(x, y, ' ');
			} else {
				draw_char(x, y, te->text[i]);
			}
			x += w;
		}
	} else {
		x = te->view_area.pos.x + te->text_area_margin + (w * te->cursor_pos_end);
		for(i=te->cursor_pos_end; i<te->cursor_pos_top+1; i++) {
			if(te->text[i] == 0) {
				draw_char(x, y, ' ');
			} else {
				draw_char(x, y, te->text[i]);
			}
			x += w;
		}
	}
}

void draw_ui_edittext(struct st_ui_edittext *textedit)
{
	struct st_ui_edittext *te = textedit;

	if(te->flg_uneditable != 0) {
		if(te->uneditable_view == 0) {
			draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te_uneditable_view);
		} else {
			draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->uneditable_view);
		}
	} else {
		if(te->flg_active == 0) {
			if(te->normal_view == 0) {
				draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te_normal_view);
			} else {
				draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->normal_view);
			}
		} else {
			if(te->active_view == 0) {
				draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te_active_view);
			} else {
				draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->active_view);
			}
		}
	}

	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(te->view_area));
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	//draw_box(&(te->view_area));
	if(te->font_name != 0) {
		(void)set_font_by_name(te->font_name);
	} else {
		(void)set_font_by_name(GSC_FONTS_DEFAULT_FONT);
	}
	draw_str(te->view_area.pos.x + te->text_area_margin, te->view_area.pos.y + te->text_area_margin, te->text, 256/* TBD */);

	if(te->flg_uneditable == 0) {
		if(te->flg_active != 0) {
			draw_cursor(te, 1);
		}
	}
}

void editable_ui_edittext(struct st_ui_edittext *textedit)
{
	if(textedit->flg_uneditable != 0) {
		textedit->flg_uneditable = 0;
		draw_ui_edittext(textedit);
	}
}

void editable_ui_edittext_list(struct st_ui_edittext **textedit)
{
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		editable_ui_edittext(*te);
		te ++;
	}
}

void uneditable_ui_edittext(struct st_ui_edittext *textedit)
{
	if(textedit->flg_uneditable == 0) {
		textedit->flg_uneditable = 1;
		textedit->flg_active = 0;
		draw_ui_edittext(textedit);
	}
}

void uneditable_ui_edittext_list(struct st_ui_edittext **textedit)
{
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		uneditable_ui_edittext(*te);
		te ++;
	}
}

void activate_ui_edittext(struct st_ui_edittext *textedit)
{
	if(textedit->flg_active == 0) {
		textedit->flg_active = 1;
		draw_ui_edittext(textedit);
	}
}

void inactivate_ui_edittext(struct st_ui_edittext *textedit)
{
	if(textedit->flg_active != 0) {
		textedit->flg_active = 0;
		draw_ui_edittext(textedit);
	}
}

void inactivate_ui_edittext_list(struct st_ui_edittext **textedit)
{
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		inactivate_ui_edittext(*te);
		te ++;
	}
}

void draw_ui_edittext_list(struct st_ui_edittext **textedit)
{
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		draw_ui_edittext(*te);
		te ++;
	}
}

static void set_cursor_pos_start(struct st_ui_edittext *textedit, short x, short y)
{
	struct st_ui_edittext *te = textedit;
	short w = font_width(' ');
	struct st_box box;
	int slen = strleng(te->text);
	int i;

	box.pos.y = te->view_area.pos.y + te->text_area_margin;
	box.sur.width = w;
	box.sur.height = font_height();

	for(i=0; i<slen; i++) {
		box.pos.x = te->view_area.pos.x + te->text_area_margin + (w * i);
		if(is_point_in_box(x, y, &box)) {
			te->cursor_pos_top = i;
			te->cursor_pos_end = i;
			break;
		}
	}

	if(slen < te->max_text_length) {
		box.pos.x = te->view_area.pos.x + te->text_area_margin + (w * (slen));
		if((te->view_area.pos.x + te->view_area.sur.width) > box.pos.x) {
			box.sur.width = (te->view_area.pos.x + te->view_area.sur.width) - box.pos.x;
			if(is_point_in_box(x, y, &box)) {
				te->cursor_pos_top = slen;
				te->cursor_pos_end = slen;
			}
		}
	}
}

static void set_cursor_pos_move(struct st_ui_edittext *textedit, short x, short y)
{
	struct st_ui_edittext *te = textedit;
	short w = font_width(' ');
	struct st_box box;
	int slen = strleng(te->text);
	int i;

	box.pos.y = te->view_area.pos.y + te->text_area_margin;
	box.sur.width = w;
	box.sur.height = font_height();

	for(i=0; i<slen; i++) {
		box.pos.x = te->view_area.pos.x + te->text_area_margin + (w * i);
		if(is_point_in_box(x, y, &box)) {
			te->cursor_pos_end = i;
			break;
		}
	}

	if(slen < te->max_text_length) {
		box.pos.x = te->view_area.pos.x + te->text_area_margin + (w * (slen));
		if((te->view_area.pos.x + te->view_area.sur.width) > box.pos.x) {
			box.sur.width = (te->view_area.pos.x + te->view_area.sur.width) - box.pos.x;
			if(is_point_in_box(x, y, &box)) {
				te->cursor_pos_end = slen;
			}
		}
	}
}

static void set_cursor_pos_end(struct st_ui_edittext *textedit, short x, short y)
{
	struct st_ui_edittext *te = textedit;
	short w = font_width(' ');
	struct st_box box;
	int slen = strleng(te->text);
	int i;

	box.pos.y = te->view_area.pos.y + te->text_area_margin;
	box.sur.width = w;
	box.sur.height = font_height();

	for(i=0; i<slen; i++) {
		box.pos.x = te->view_area.pos.x + te->text_area_margin + (w * i);
		if(is_point_in_box(x, y, &box)) {
			te->cursor_pos_end = i;
			break;
		}
	}

	if(slen < te->max_text_length) {
		box.pos.x = te->view_area.pos.x + te->text_area_margin + (w * (slen));
		if((te->view_area.pos.x + te->view_area.sur.width) > box.pos.x) {
			box.sur.width = (te->view_area.pos.x + te->view_area.sur.width) - box.pos.x;
			if(is_point_in_box(x, y, &box)) {
				te->cursor_pos_end = slen;
			}
		}
	}

	DTPRINTF(0x01, "STOP=%d, SEND=%d\n", te->cursor_pos_top, te->cursor_pos_end);
	if(te->cursor_pos_end < te->cursor_pos_top) {
		i = te->cursor_pos_top;
		te->cursor_pos_top = te->cursor_pos_end;
		te->cursor_pos_end = i;
	}
	DTPRINTF(0x01, "TOP=%d, END=%d\n", te->cursor_pos_top, te->cursor_pos_end);
}

int proc_ui_edittext(struct st_ui_edittext *textedit, struct st_sysevent *event)
{
	struct st_ui_edittext *te = textedit;
	int rtn = UI_EDITTEXT_EVT_NULL;

	switch(event->what) {
	case EVT_TOUCHSTART:
		if(te->flg_uneditable != 0) {
			rtn = UI_EDITTEXT_EVT_NULL;
		} else if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(te->view_area)) != 0) {
			set_cursor_pos_start(te, event->pos_x, event->pos_y);
			if(te->flg_active == 0) {
				te->flg_active = 1;
				rtn = UI_EDITTEXT_EVT_PUSH;
			} else {
				rtn = UI_EDITTEXT_EVT_NULL;
			}
			draw_ui_edittext(te);
		}
		break;

	case EVT_TOUCHMOVE:
		if(te->flg_uneditable != 0) {
			rtn = UI_EDITTEXT_EVT_NULL;
		} else if(te->flg_enable_multibyte_select == 0) {
			rtn = UI_EDITTEXT_EVT_NULL;
		} else if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(te->view_area)) != 0) {
			// イベント位置がが範囲内
			set_cursor_pos_move(te, event->pos_x, event->pos_y);
			if(te->flg_active != 0) {
				rtn = UI_EDITTEXT_EVT_DRAG;
			} else {
				rtn = UI_EDITTEXT_EVT_NULL;
			}
			draw_ui_edittext(te);
		} else {
			// イベント位置がが範囲外
			rtn = UI_EDITTEXT_EVT_NULL;
		}
		break;

	case EVT_TOUCHEND:
		if(te->flg_uneditable != 0) {
			rtn = UI_EDITTEXT_EVT_NULL;
		} else if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(te->view_area)) != 0) {
			// イベント位置がが範囲内
			if(te->flg_enable_multibyte_select != 0) {
				set_cursor_pos_end(te, event->pos_x, event->pos_y);
				draw_ui_edittext(te);
			}
			if(te->flg_active != 0) {
				rtn = UI_EDITTEXT_EVT_RELEASE;
			} else {
				rtn = UI_EDITTEXT_EVT_NULL;
			}
		} else {
			// イベント位置がが範囲外
			if(te->flg_active != 0) {
				rtn = UI_EDITTEXT_EVT_RELEASE;
			}
		}
		break;

	default:
		break;
	}

	return rtn;
}

int proc_ui_edittext_list(struct st_ui_edittext **textedit, struct st_sysevent *event, struct st_ui_edittext **last_te)
{
	struct st_ui_edittext **te = textedit;
	int flg_evt = 0;
	struct st_ui_edittext *act_te = 0;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			act_te = *te;
		}
		te ++;
	}

	if(*last_te != 0) {
		int obj_ev = proc_ui_edittext(*last_te, event);
		if(obj_ev == UI_EDITTEXT_EVT_RELEASE) {
			// カーソル設定終了
			*last_te = 0;
		}
	} else {
		te = textedit;
		while(*te != 0) {
			int obj_ev = proc_ui_edittext(*te, event);
			if(obj_ev == UI_EDITTEXT_EVT_PUSH) {
				// カーソル設定開始
				*last_te = *te;
				if(act_te != 0) {
					if(act_te != *te) {
						inactivate_ui_edittext(act_te);
					}
				}
				flg_evt = 1;
				goto end;
			}
			te ++;
		}
	}

end:
	return flg_evt;
}

int set_char_ui_edittext(struct st_ui_edittext *textedit, unsigned char ch)
{
	struct st_ui_edittext *te = textedit;
	int rt = 0;

	if(te->flg_uneditable != 0) {
		return 0;
	}

	if(te->filter != 0) {
		int frt = te->filter(te, ch);
		if(frt != 0) {
			return 0;
		}
	}

	if(te->cursor_pos_top == te->cursor_pos_end) {
		te->text[te->cursor_pos_top] = ch;
		draw_cursor(te, 0);
	} else {
		int i, j;
		int len = strleng(te->text);

		te->text[te->cursor_pos_top] = ch;

		for(i=te->cursor_pos_end, j=0; i<(len - 1); i++, j++) {
			te->text[te->cursor_pos_top + 1 + j] = te->text[i];
		}
		for(; j<te->max_text_length; j++) {
			te->text[te->cursor_pos_top + 1 + j] = 0;
		}
		draw_ui_edittext(te);
		draw_cursor(te, 0);
	}

	te->cursor_pos_top ++;
	if(te->cursor_pos_top >= te->max_text_length) {
		te->cursor_pos_top = 0;
		te->cursor_pos_end = 0;
		inactivate_ui_edittext(te);
		rt = 1;
	} else {
		te->cursor_pos_end = te->cursor_pos_top;
		draw_cursor(te, 1);
	}

	return rt;
}

int set_char_ui_edittext_list(struct st_ui_edittext **textedit, unsigned char ch, int all_select)
{
	int rt = 0;
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			rt = set_char_ui_edittext(*te, ch);
			if(rt != 0) {
				te ++;
				if((*te) != 0) {
					if(all_select != 0) {
						select_all_ui_edittext(*te);
					} else {
						(*te)->cursor_pos_top = 0;
						(*te)->cursor_pos_end = 0;
					}
					activate_ui_edittext(*te);
				}
			}
			break;
		}
		te ++;
	}

	return rt;
}

int move_cursor_ui_edittext(struct st_ui_edittext *textedit, int direction)
{
	struct st_ui_edittext *te = textedit;
	int rt = 0;

	if(direction == 0) {
		return rt;
	}

	draw_cursor(te, 0);

	if(direction > 0) {
		if(te->text[te->cursor_pos_end] == 0) {
			inactivate_ui_edittext(te);
			rt = 1;
		} else {
			te->cursor_pos_end ++;
			if(te->cursor_pos_end >= te->max_text_length) {
				te->cursor_pos_end = (te->max_text_length - 1);
				te->cursor_pos_top = te->cursor_pos_end;
				inactivate_ui_edittext(te);
				rt = 1;
			} else {
				te->cursor_pos_top = te->cursor_pos_end;
				draw_cursor(te, 1);
			}
		}
	} else if(direction < 0) {
		te->cursor_pos_top --;
		if(te->cursor_pos_top < 0) {
			te->cursor_pos_top = 0;
			te->cursor_pos_end = te->cursor_pos_top;
			inactivate_ui_edittext(te);
			rt = -1;
		} else {
			te->cursor_pos_end = te->cursor_pos_top;
			draw_cursor(te, 1);
		}
	}

	return rt;
}

int move_cursor_ui_edittext_list(struct st_ui_edittext **textedit, int direction, int all_select)
{
	int rt = 0;
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			rt = move_cursor_ui_edittext(*te, direction);
			if(rt > 0) {
				te ++;
				if((*te) != 0) {
					if(all_select != 0) {
						select_all_ui_edittext(*te);
					} else {
						(*te)->cursor_pos_top = 0;
						(*te)->cursor_pos_end = 0;
					}
					activate_ui_edittext(*te);
				} else {
					activate_ui_edittext(*(te-1));
				}
			} else if(rt < 0) {
				if(te != textedit) {
					te --;
					if((*te) != 0) {
						int i;
						for(i=((*te)->max_text_length)-1; i>0; i--) {
							if((*te)->text[i-1] != 0) {
								break;
							}
						}
						(*te)->cursor_pos_top = i;
						(*te)->cursor_pos_end = i;
						activate_ui_edittext(*te);
					}
				} else {
					(*te)->cursor_pos_top = 0;
					(*te)->cursor_pos_end = 0;
					activate_ui_edittext(*te);
				}
			}
			break;
		}
		te ++;
	}

	return rt;
}

void select_all_ui_edittext(struct st_ui_edittext *textedit)
{
	struct st_ui_edittext *te = textedit;
	int len = 0;

	te->cursor_pos_top = 0;
	len = strleng(te->text);
	if(len == 0) {
		te->cursor_pos_end = 0;
	} else {
		te->cursor_pos_end = len - 1;
	}
	te->flg_active = 1;

	draw_ui_edittext(te);
}

void unactive_ui_edittext(struct st_ui_edittext *textedit)
{
	if(textedit->flg_active != 0) {
		textedit->flg_active = 0;
		draw_ui_edittext(textedit);
	}
}

void unactive_ui_edittext_list(struct st_ui_edittext **textedit)
{
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		unactive_ui_edittext(*te);
		te ++;
	}
}

static void delete_char(uchar *str, int maxlen, int start, int end);

static void backspace_char(uchar *str, int maxlen, int start, int end)
{
	int len = strleng(str);
	int i;

	DTFPRINTF(0x02, "str=\"%s\", maxlen=%d, len=%d, start=%d, end=%d\n", str, maxlen, len, start, end);

	if(start < end) {
		delete_char(str, maxlen, start, end);
	} else if(start == 0) {
		return;
	} else {
		for(i=(start-1); i<len; i++) {
			str[i] = str[i+1];
		}
		str[len] = 0;
	}

	XDUMP(0x02, str, maxlen);
	DTFPRINTF(0x02, "str=\"%s\"\n", str);
}

int backspace_char_ui_edittext(struct st_ui_edittext *textedit)
{
	struct st_ui_edittext *te = textedit;

	if(te->flg_uneditable != 0) {
		return 0;
	}

	if(te->flg_active == 0) {
		return 0;
	}

	if(te->filter != 0) {
		int frt = te->filter(te, 0);
		if(frt != 0) {
			return 0;
		}
	}

	backspace_char(te->text, te->max_text_length, te->cursor_pos_top, te->cursor_pos_end);
	te->cursor_pos_top --;
	if(te->cursor_pos_top < 0) {
		te->cursor_pos_top = 0;
	}
	te->cursor_pos_end = te->cursor_pos_top;
	draw_ui_edittext(te);
	draw_cursor(te, 1);

	return 1;
}

int backspace_char_ui_edittext_list(struct st_ui_edittext **textedit)
{
	int rt = 0;
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			rt = backspace_char_ui_edittext(*te);
			break;
		}
		te ++;
	}

	return rt;
}

static void delete_char(uchar *str, int maxlen, int start, int end)
{
	int len = strleng(str);
	int i, j;

	DTFPRINTF(0x02, "str=\"%s\", maxlen=%d, len=%d, start=%d, end=%d\n", str, maxlen, len, start, end);

	if(start >= len) {
		return;
	}

	for(i=start,j=0; i<len; i++,j++) {
		int src = end+1+j;
		if(src < maxlen) {
			str[i] = str[src];
		} else {
			str[i] = 0;
		}
	}

	XDUMP(0x02, str, maxlen);
	DTFPRINTF(0x02, "str=\"%s\"\n", str);
}

int delete_char_ui_edittext(struct st_ui_edittext *textedit)
{
	struct st_ui_edittext *te = textedit;

	if(te->flg_uneditable != 0) {
		return 0;
	}

	if(te->flg_active == 0) {
		return 0;
	}

	if(te->filter != 0) {
		int frt = te->filter(te, 0);
		if(frt != 0) {
			return 0;
		}
	}

	delete_char(te->text, te->max_text_length, te->cursor_pos_top, te->cursor_pos_end);
	te->cursor_pos_end = te->cursor_pos_top;
	draw_ui_edittext(te);
	draw_cursor(te, 1);

	return 1;
}

int delete_char_ui_edittext_list(struct st_ui_edittext **textedit)
{
	int rt = 0;
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			rt = delete_char_ui_edittext(*te);
			break;
		}
		te ++;
	}

	return rt;
}

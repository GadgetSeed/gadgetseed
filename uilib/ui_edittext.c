/** @file
    @brief	ユーザインタフェース - テキストエディット

    @date	2018.01.29
    @auther	Takashi SHUDO
*/

#include "font.h"
#include "ui_edittext.h"

static void draw_cursor(struct st_ui_edittext *te, int flg_on)
{
	short x, y;
	short w = font_width(' ');

	y = te->view_area.pos.y + TEXT_AREA_MARGINE;
	x = te->view_area.pos.x + TEXT_AREA_MARGINE + (w * te->cursor_pos);

	if(flg_on != 0) {
		draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->cursor_view);
	} else {
		draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->active_view);
	}

	draw_char(x, y, te->text[te->cursor_pos]);
}

void draw_ui_edittext(struct st_ui_edittext *textedit)
{
	struct st_ui_edittext *te = textedit;

	if(te->flg_active == 0) {
		draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->normal_view);
	} else {
		draw_graph_object(te->view_area.pos.x, te->view_area.pos.y, te->active_view);
	}

	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(te->view_area));
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	//draw_box(&(te->view_area));
	draw_str(te->view_area.pos.x + TEXT_AREA_MARGINE, te->view_area.pos.y + TEXT_AREA_MARGINE, te->text);

	if(te->flg_active != 0) {
		draw_cursor(te, 1);
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

static void set_cursor_pos(struct st_ui_edittext *textedit, short x, short y)
{
	struct st_ui_edittext *te = textedit;
	short w = font_width(' ');
	struct st_box box;
	int slen = strleng(te->text);
	int i;

	box.pos.y = te->view_area.pos.y + TEXT_AREA_MARGINE;
	box.sur.width = w;
	box.sur.height = font_height();

	for(i=0; i<slen; i++) {
		box.pos.x = te->view_area.pos.x + TEXT_AREA_MARGINE + (w * i);
		if(is_point_in_box(x, y, &box)) {
			te->cursor_pos = i;
			break;
		}
	}
}

int proc_ui_edittext(struct st_ui_edittext *textedit, struct st_sysevent *event)
{
	struct st_ui_edittext *te = textedit;
	int rtn = 0;

	switch(event->what) {
	case EVT_TOUCHSTART:
		if(is_point_in_box(event->pos_x, event->pos_y,
				   (struct st_box *)&(te->view_area)) != 0) {
			set_cursor_pos(te, event->pos_x, event->pos_y);
			if(te->flg_active == 0) {
				te->flg_active = 1;
				rtn = 1;
			} else {
				rtn = 0;
			}
			draw_ui_edittext(te);
		}
		break;

	default:
		break;
	}

	return rtn;
}

int proc_ui_edittext_list(struct st_ui_edittext **textedit, struct st_sysevent *event)
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

	te = textedit;
	while(*te != 0) {
		int obj_ev = proc_ui_edittext(*te, event);
		if(obj_ev != 0) {
			if(act_te != 0) {
				act_te->flg_active = 0;
				draw_ui_edittext(act_te);
			}
		}
		te ++;
	}

	return flg_evt;
}

int set_char_ui_edittext(struct st_ui_edittext *textedit, unsigned char ch)
{
	struct st_ui_edittext *te = textedit;
	int rt = 0;

	if(te->filter != 0) {
		int frt = te->filter(te, ch);
		if(frt != 0) {
			return 0;
		}
	}

	te->text[te->cursor_pos] = ch;
	draw_cursor(te, 0);

	te->cursor_pos ++;
	if(te->cursor_pos >= te->max_text_length) {
		te->cursor_pos = 0;
		te->flg_active = 0;
		draw_ui_edittext(te);
		rt = 1;
	} else {
		draw_cursor(te, 1);
	}

	return rt;
}

int set_char_ui_edittext_list(struct st_ui_edittext **textedit, unsigned char ch)
{
	int rt = 0;
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			rt = set_char_ui_edittext(*te, ch);
			if(rt != 0) {
				te ++;
				if((*te) != 0) {
					(*te)->flg_active = 1;
					(*te)->cursor_pos = 0;
					draw_ui_edittext(*te);
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
		te->cursor_pos ++;
		if(te->cursor_pos >= te->max_text_length) {
			te->cursor_pos = (te->max_text_length - 1);
			te->flg_active = 0;
			draw_ui_edittext(te);
			rt = 1;
		} else {
			draw_cursor(te, 1);
		}
	} else if(direction < 0) {
		te->cursor_pos --;
		if(te->cursor_pos < 0) {
			te->cursor_pos = 0;
			te->flg_active = 0;
			draw_ui_edittext(te);
			rt = -1;
		} else {
			draw_cursor(te, 1);
		}
	}

	return rt;
}

int move_cursor_ui_edittext_list(struct st_ui_edittext **textedit, int direction)
{
	int rt = 0;
	struct st_ui_edittext **te = textedit;

	while(*te != 0) {
		if((*te)->flg_active != 0) {
			rt = move_cursor_ui_edittext(*te, direction);
			if(rt > 0) {
				te ++;
				if((*te) != 0) {
					(*te)->flg_active = 1;
					(*te)->cursor_pos = 0;
					draw_ui_edittext(*te);
				}
			} else if(rt < 0) {
				if(te != textedit) {
					te --;
					if((*te) != 0) {
						(*te)->flg_active = 1;
						(*te)->cursor_pos = ((*te)->max_text_length - 1);
						draw_ui_edittext(*te);
					}
				} else {
					(*te)->flg_active = 1;
					(*te)->cursor_pos = 0;
					draw_ui_edittext(*te);
				}
			}
			break;
		}
		te ++;
	}

	return rt;
}

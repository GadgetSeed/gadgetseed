/** @file
    @brief	時間設定ダイアログボックス

    @date	2018.01.27
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "graphics.h"
#include "font.h"
#include "graphics_object.h"

#include "ui_style.h"
#include "ui_button.h"
#include "ui_edittext.h"
#include "tenkey.h"

#include "timeset.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static struct st_rect setting_view_area = {
	0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT
};

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define TIMESET_FONT	"num48x64"
#define EDITCHAR_WIDTH	48
#define EDITCHAR_HEIGHT	64
#elif GSC_GRAPHICS_DISPLAY_WIDTH >= 480
#define TIMESET_FONT	"num24x48"
#define EDITCHAR_WIDTH	24
#define EDITCHAR_HEIGHT	48
#elif GSC_GRAPHICS_DISPLAY_WIDTH >= 320
#define TIMESET_FONT	"16x24"
#define EDITCHAR_WIDTH	16
#define EDITCHAR_HEIGHT	24
#endif

#if GSC_GRAPHICS_DISPLAY_HEIGHT == 480
#define PANEL_Y		40
#define BUTTON_TOP	0
#define BUTTON_HEIGHT	64
#define TIME_TEXT_Y	16
#define TIME_ITEM_HEIGHT	88
#define TEXT_AREA_MARGIN	4
#define BTN_POS_Y_SET	372
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 320
#define PANEL_Y		24
#define BUTTON_TOP	24
#define BUTTON_HEIGHT	48
#define TIME_TEXT_Y	0
#define TIME_ITEM_HEIGHT	64
#define TEXT_AREA_MARGIN	3
#define BTN_POS_Y_SET	244
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 272
#define PANEL_Y		0
#define BUTTON_TOP	0
#define BUTTON_HEIGHT	48
#define TIME_TEXT_Y	16
#define TIME_ITEM_HEIGHT	64
#define TEXT_AREA_MARGIN	3
#define BTN_POS_Y_SET	(GSC_GRAPHICS_DISPLAY_HEIGHT - BUTTON_HEIGHT - BUTTON_INTERVAL)
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 240
#define PANEL_Y		0
#define BUTTON_TOP	0
#define BUTTON_HEIGHT	24
#define TIME_TEXT_Y	48
#define TIME_ITEM_HEIGHT	46
#define TEXT_AREA_MARGIN	2
#define BTN_POS_Y_SET	(GSC_GRAPHICS_DISPLAY_HEIGHT - BUTTON_HEIGHT - BUTTON_INTERVAL)
#endif

#if GSC_GRAPHICS_DISPLAY_WIDTH == 320
#define PANEL_X		0
#define BUTTON_WIDTH	64
#define TIME_TEXT_X	8
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 480
#define PANEL_X		8
#define BUTTON_WIDTH	96
#define TIME_TEXT_X	8
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 800
#define PANEL_X		16
#define BUTTON_WIDTH	128
#define TIME_TEXT_X	8
#else
#define PANEL_X		0
#define BUTTON_WIDTH	48
#define TIME_TEXT_X	8
#endif

#define BUTTON_LEFT	160
#define BUTTON_INTERVAL	4

#define SET_BUTTON_WIDTH	BUTTON_WIDTH
#define CANCEL_BUTTON_WIDTH	BUTTON_WIDTH

#define BUTTON_X	16

#define BTN_POS_X_SET	(PANEL_X + BUTTON_X + SET_BUTTON_WIDTH + (BUTTON_INTERVAL*2))

#define BTN_POS_X_CANCEL	(PANEL_X + BUTTON_X)
#define BTN_POS_Y_CANCEL	(BTN_POS_Y_SET)

#define BTN_ID_SET	12
#define BTN_ID_CANCEL	11

static struct st_ui_button ui_btn_set = {
	.id	= BTN_ID_SET,
	.view_area	= { { BTN_POS_X_SET,  BTN_POS_Y_CANCEL}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.name	= "SET",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_cancel = {
	.id	= BTN_ID_CANCEL,
	.view_area	= { {BTN_POS_X_CANCEL,  BTN_POS_Y_CANCEL}, {CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT} },
	.name	= "CANCEL",
	.status	= UI_BUTTON_ST_NORMAL,
};


static struct st_ui_button *ui_timeset_button_view[] = {
	&ui_btn_set,
	&ui_btn_cancel,
	0
};

static unsigned char str_year[5];
static unsigned char str_month[3];
static unsigned char str_day[3];
static unsigned char str_hour[3];
static unsigned char str_min[3];
static unsigned char str_sec[3];

#define TIME_YEAR_Y		(TIME_TEXT_Y + TIME_ITEM_HEIGHT*0)
#define TIME_MONTHDATE_Y	(TIME_TEXT_Y + TIME_ITEM_HEIGHT*1)
#define TIME_HOURMINSEC_Y	(TIME_TEXT_Y + TIME_ITEM_HEIGHT*2)

//#define TEXT_AREA_MARGIN	6

static struct st_ui_edittext te_year = {
	.view_area = {
		.pos.x	= PANEL_X + TIME_TEXT_X,
		.pos.y	= PANEL_Y + TIME_YEAR_Y,
		.sur.width	= (EDITCHAR_WIDTH*4) + (TEXT_AREA_MARGIN * 2),
		.sur.height	= EDITCHAR_HEIGHT + (TEXT_AREA_MARGIN * 2),
	},
	.text_area_margin	= TEXT_AREA_MARGIN,
	.font_name	= TIMESET_FONT,
	.text		= str_year,
	.max_text_length = 4,
};

static int month_filter(struct st_ui_edittext *te, unsigned char ch)
{
	if(te->cursor_pos_top == 0) {
		if((ch == '0') || (ch == '1')) {
			// OK
		} else {
			return 1;
		}
	} else if(te->cursor_pos_top == 1) {
		if(te->text[0] == '1') {
			if((ch >= '0') && (ch <= '2')) {
				// OK
			} else {
				return 1;
			}
		} else if(te->text[0] == '0') {
			if((ch >= '1') && (ch <= '9')) {
				// OK
			} else {
				return 1;
			}
		}
	}

	return 0;
}

static struct st_ui_edittext te_month = {
	.view_area = {
		.pos.x	= PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 0),
		.pos.y	= PANEL_Y + TIME_MONTHDATE_Y,
		.sur.width	= (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2),
		.sur.height	= EDITCHAR_HEIGHT + (TEXT_AREA_MARGIN * 2),
	},
	.text_area_margin	= TEXT_AREA_MARGIN,
	.font_name	= TIMESET_FONT,
	.text		= str_month,
	.max_text_length = 2,
	.filter		= month_filter,
};

static int day_filter(struct st_ui_edittext *te, unsigned char ch)
{
	int year = dstoi(str_year);
	int month = dstoi(str_month);
	int nday = num_of_day_in_month(year, month);

	DTPRINTF(0x01, "%d/%d = %d days\n", year, month, nday);

	if(te->cursor_pos_top == 0) {
		if(ch <= ('0' + (nday/10))) {
			// OK
		} else {
			return 1;
		}
	} else if(te->cursor_pos_top == 1) {
		if(te->text[0] == '0') {
			if(ch == '0') {
				return 1;
			}
		} else {
			if(month != 2) {
				if(te->text[0] == '3') {
					if(ch <= ('0' + (nday % 10))) {
						// OK
					} else {
						return 1;
					}
				}
			} else {
				if(te->text[0] == '2') {
					if(ch <= ('0' + (nday % 10))) {
						// OK
					} else {
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

static struct st_ui_edittext te_day = {
//	.view_area	= { {PANEL_X + 8 + 16*4, PANEL_Y + 16 + 52}, {(EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2), 32} },
	.view_area = {
		.pos.x	= PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 3) + (TEXT_AREA_MARGIN * 2),
		.pos.y	= PANEL_Y + TIME_MONTHDATE_Y,
		.sur.width	= (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2),
		.sur.height	= EDITCHAR_HEIGHT + (TEXT_AREA_MARGIN * 2),
	},
	.text_area_margin	= TEXT_AREA_MARGIN,
	.font_name	= TIMESET_FONT,
	.text		= str_day,
	.max_text_length = 2,
	.filter		= day_filter,
};

static int hour_filter(struct st_ui_edittext *te, unsigned char ch)
{
	if(te->cursor_pos_top == 0) {
		if((ch >= '0') && (ch <= '2')) {
			// OK
		} else {
			return 1;
		}
	} else if(te->cursor_pos_top == 1) {
		if(te->text[0] == '2') {
			if((ch >= '0') && (ch <= '3')) {
				// OK
			} else {
				return 1;
			}
		}
	}

	return 0;
}

static struct st_ui_edittext te_hour = {
//	.view_area	= { {PANEL_X + 8, PANEL_Y + 16 + 52*2}, {(EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2), 32} },
	.view_area = {
		.pos.x	= PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 0),
		.pos.y	= PANEL_Y + TIME_HOURMINSEC_Y,
		.sur.width	= (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2),
		.sur.height	= EDITCHAR_HEIGHT + (TEXT_AREA_MARGIN * 2),
	},
	.text_area_margin	= TEXT_AREA_MARGIN,
	.font_name	= TIMESET_FONT,
	.text		= str_hour,
	.max_text_length = 2,
	.filter		= hour_filter,
};

static int minsec_filter(struct st_ui_edittext *te, unsigned char ch)
{
	if(te->cursor_pos_top == 0) {
		if((ch >= '0') && (ch <= '5')) {
			// OK
		} else {
			return 1;
		}
	}

	return 0;
}

static struct st_ui_edittext te_min = {
//	.view_area	= { {PANEL_X + 8 + 50, PANEL_Y + 16 + 52*2}, {(EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2), 32} },
	.view_area = {
		.pos.x	= PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 3) + (TEXT_AREA_MARGIN * 2),
		.pos.y	= PANEL_Y + TIME_HOURMINSEC_Y,
		.sur.width	= (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2),
		.sur.height	= EDITCHAR_HEIGHT + (TEXT_AREA_MARGIN * 2),
	},
	.text_area_margin	= TEXT_AREA_MARGIN,
	.font_name	= TIMESET_FONT,
	.text		= str_min,
	.max_text_length = 2,
	.filter		= minsec_filter,
};

static struct st_ui_edittext te_sec = {
//	.view_area	= { {PANEL_X + 8 + 100, PANEL_Y + 16 + 52*2}, {(EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2), 32} },
	.view_area = {
		.pos.x	= PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 6) + (TEXT_AREA_MARGIN * 4),
		.pos.y	= PANEL_Y + TIME_HOURMINSEC_Y,
		.sur.width	= (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2),
		.sur.height	= EDITCHAR_HEIGHT + (TEXT_AREA_MARGIN * 2),
	},
	.text_area_margin	= TEXT_AREA_MARGIN,
	.font_name	= TIMESET_FONT,
	.text		= str_sec,
	.max_text_length = 2,
	.filter		= minsec_filter,
};

static struct st_ui_edittext *settime_textedit[] = {
	&te_year,
	&te_month,
	&te_day,
	&te_hour,
	&te_min,
	&te_sec,
	0
};

static struct st_ui_edittext *last_te = 0;
static int flg_setting = 0;
static struct st_systime last_time;
static struct st_datetime last_datetime;


static void prepare_te(struct st_ui_edittext *te)
{
	te->flg_active = 0;
	te->cursor_pos_top = 0;
	te->cursor_pos_end = 0;
}

static void prepare_timestr(struct st_datetime *datetime)
{
	char date_str[DATE_STR_LEN + 1];
	char time_str[TIME_STR_LEN + 1];

	date_to_str(date_str, datetime);
	strncopy((uchar *)str_year, (const uchar *)&date_str[0], 4);
	strncopy((uchar *)str_month, (const uchar *)&date_str[5], 2);
	strncopy((uchar *)str_day, (const uchar *)&date_str[8], 2);

	time_to_str(time_str, datetime);
	strncopy((uchar *)str_hour, (const uchar *)&time_str[0], 2);
	strncopy((uchar *)str_min, (const uchar *)&time_str[3], 2);
	strncopy((uchar *)str_sec, (const uchar *)&time_str[6], 2);

	prepare_te(&te_year);
	prepare_te(&te_min);
	prepare_te(&te_day);
	prepare_te(&te_hour);
	prepare_te(&te_min);
	prepare_te(&te_sec);
}

void prepare_timeset(struct st_datetime *datetime)
{
	last_datetime = *datetime;

	prepare_timestr(datetime);

	get_systime(&last_time);

	flg_setting = 0;
}

void draw_timeset(void)
{
	set_mode_tenkey(0);
	set_forecolor(UI_BACK_COLOR);
	draw_fill_rect(&setting_view_area);

	set_forecolor(UI_NORMAL_FORE_COLOR);
	set_backcolor(UI_BACK_COLOR);
	set_font_by_name(TIMESET_FONT);
	set_graph_obj_scale(1, 1);

	draw_char(PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2), te_month.view_area.pos.y + TEXT_AREA_MARGIN, '/');
	draw_char(PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 2) + (TEXT_AREA_MARGIN * 2), te_hour.view_area.pos.y + TEXT_AREA_MARGIN, ':');
	draw_char(PANEL_X + TIME_TEXT_X + (EDITCHAR_WIDTH * 5) + (TEXT_AREA_MARGIN * 4), te_hour.view_area.pos.y + TEXT_AREA_MARGIN, ':');

	draw_ui_button_list(ui_timeset_button_view);

	draw_ui_edittext_list(settime_textedit);

	draw_tenkey();
}

static void set_datetime(void)
{
	struct st_datetime ndate;
	struct st_systime systime;

	ndate.year	= dstoi(str_year);
	ndate.month	= dstoi(str_month);
	ndate.day	= dstoi(str_day);
	ndate.hour	= dstoi(str_hour);
	ndate.min	= dstoi(str_min);
	ndate.sec	= dstoi(str_sec);
	ndate.msec	= 0;
	ndate.dayofweek = date_to_dayofweek(ndate.year, ndate.month, ndate.day);

	DTPRINTF(0x01, "%4d/%02d/%02d %02d:%02d:%02d\n",
		 ndate.year,
		 ndate.month,
		 ndate.day,
		 ndate.hour,
		 ndate.min,
		 ndate.sec);

	systime.sec = datetime_to_utc(&ndate);
	systime.usec = 0;
	set_systime(&systime);
}

int proc_timesetting(struct st_sysevent *event)
{
	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_timeset_button_view, event) != 0) {
		switch(obj_evt.id) {
		case BTN_ID_SET:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				set_datetime();
				return 1;
			}
			break;

		case BTN_ID_CANCEL:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				return -1;
			}
			break;

		default:
			break;
		}
	}

	if(proc_ui_edittext_list(settime_textedit, event, &last_te) != 0) {
		// [TODO]
	}

	if(proc_tenkey(&obj_evt, event) != 0) {
		switch(obj_evt.id) {
		case BTN_ID_0:
		case BTN_ID_1:
		case BTN_ID_2:
		case BTN_ID_3:
		case BTN_ID_4:
		case BTN_ID_5:
		case BTN_ID_6:
		case BTN_ID_7:
		case BTN_ID_8:
		case BTN_ID_9:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				set_char_ui_edittext_list(settime_textedit, '0' + obj_evt.id - BTN_ID_0, 0);
			}
			break;

		case BTN_ID_BACK:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				move_cursor_ui_edittext_list(settime_textedit, -1, 0);
			}
			break;

		case BTN_ID_FORWARD:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				move_cursor_ui_edittext_list(settime_textedit, 1, 0);
			}
			break;

		case BTN_ID_BACKSPACE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				backspace_char_ui_edittext_list(settime_textedit);
			}
			break;

		case BTN_ID_DELETE:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				delete_char_ui_edittext_list(settime_textedit);
			}
			break;

		default:
			break;
		}
	}

	return 0;
}

int proc_timedisp(struct st_sysevent *event)
{
	struct st_systime now_time;
	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_timeset_button_view, event) != 0) {
		switch(obj_evt.id) {
		case BTN_ID_SET:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				return 1;
			}
			break;

		case BTN_ID_CANCEL:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				return -1;
			}
			break;

		default:
			break;
		}
	}

	if(proc_ui_edittext_list(settime_textedit, event, &last_te) != 0) {
		flg_setting = 1;
		return 0;
	}

	get_systime(&now_time);

	if(last_time.sec != now_time.sec) {
		struct st_datetime now_datetime;

		last_time = now_time;
		systime_to_datetime(&now_datetime, &now_time);

		prepare_timestr(&now_datetime);

		if(last_datetime.year != now_datetime.year) {
			draw_ui_edittext(&te_year);
		}
		if(last_datetime.month != now_datetime.month) {
			draw_ui_edittext(&te_month);
		}
		if(last_datetime.day != now_datetime.day) {
			draw_ui_edittext(&te_day);
		}
		if(last_datetime.hour != now_datetime.hour) {
			draw_ui_edittext(&te_hour);
		}
		if(last_datetime.min != now_datetime.min) {
			draw_ui_edittext(&te_min);
		}
		if(last_datetime.sec != now_datetime.sec) {
			draw_ui_edittext(&te_sec);
		}

		last_datetime = now_datetime;
	}

	return 0;
}

int proc_timeset(struct st_sysevent *event)
{
	int rt = 0;

	if(flg_setting == 0) {
		rt = proc_timedisp(event);
	} else {
		rt = proc_timesetting(event);
	}

	return rt;
}

int do_timeet(timeset_proc proc)
{
	while(1) {
		struct st_sysevent event;
		int rt = 0;

		get_event(&event, 50);
		rt = proc_timeset(&event);
		if(rt != 0) {
			return rt;
		}
		if(proc !=0) {
			rt = proc(&event);
		}
		if(rt != 0) {
			return rt;
		}
	}

	return 0;
}

int open_timeset_dialog(timeset_proc proc)
{
	int rt = 0;

	draw_timeset();

	rt = do_timeet(proc);

	return rt;
}

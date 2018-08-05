/** @file
    @brief	時間設定インタフェース

    @date	2018.01.27
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "graphics.h"
#include "font.h"
#include "graphics_object.h"
#include "timeset.h"
#include "clock.h"
#include "ui_button.h"
#include "ui_edittext.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static struct st_rect setting_view_area = {
	0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT
};

#define BUTTON_FONT	"16x24"

#if GSC_GRAPHICS_DISPLAY_HEIGHT == 480
#define PANEL_Y		40
#define BUTTON_TOP	0
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 272
#define PANEL_Y		0
#define BUTTON_TOP	0
#else
#define PANEL_Y		0
#define BUTTON_TOP	24
#endif

#if GSC_GRAPHICS_DISPLAY_WIDTH == 320
#define PANEL_X		0
#define BUTTON_WIDTH	48
#define BUTTON_HEIGHT	48
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 480
#define PANEL_X	60
#define BUTTON_WIDTH	64
#define BUTTON_HEIGHT	64
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 800
#define PANEL_X	180
#define BUTTON_WIDTH	96
#define BUTTON_HEIGHT	96
#else
#define PANEL_X		0
#define BUTTON_WIDTH	48
#define BUTTON_HEIGHT	48
#endif

#define BUTTON_LEFT	160
#define BUTTON_INTERVAL	4

#define SET_BUTTON_WIDTH	(48 * 2)
#define CANCEL_BUTTON_WIDTH	(48 * 2 + 8)

#define BTN_POS_X_7	(PANEL_X + BUTTON_LEFT)
#define BTN_POS_Y_7	(PANEL_Y + BUTTON_TOP)

#define BTN_POS_X_8	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_8	(PANEL_Y + BUTTON_TOP)

#define BTN_POS_X_9	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_9	(PANEL_Y + BUTTON_TOP)

#define BTN_POS_X_4	(PANEL_X + BUTTON_LEFT)
#define BTN_POS_Y_4	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_5	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_5	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_6	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_6	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_1	(PANEL_X + BUTTON_LEFT)
#define BTN_POS_Y_1	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_2	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_2	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_3	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_3	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_SET	(PANEL_X + 4)
#define BTN_POS_Y_SET	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_CANCEL	(PANEL_X + 4 + SET_BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_CANCEL	(BTN_POS_Y_SET)

#define BTN_POS_X_0	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_0	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_BACK	(PANEL_X + BUTTON_LEFT + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_BACK	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define NUM_OBJ(X, C)				  \
	{ GO_TYPE_FORECOLOR,	{ (C) } },			\
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },\
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } }, \
	{ GO_TYPE_BACKCOLOR,	{ (C) } },			\
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } }, \
	{ GO_TYPE_FONT,		{ 0 }, (void *)BUTTON_FONT },		\
	{ GO_TYPE_TEXT_IN_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT, FONT_HATTR_CENTER, FONT_VATTR_CENTER }, (void *)(X) }, \
	{ 0, { 0, 0, 0, 0 }}

static const struct st_graph_object obj_button_set[] = {
	{ GO_TYPE_FORECOLOR,	{ BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, SET_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, SET_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FONT,		{ 0 }, (void *)BUTTON_FONT },
	{ GO_TYPE_TEXT_IN_BOX,	{ 0, 0, SET_BUTTON_WIDTH, BUTTON_HEIGHT, FONT_HATTR_CENTER, FONT_VATTR_CENTER }, (void *)"SET" },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_set_a[] = {
	{ GO_TYPE_FORECOLOR,	{ ACT_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, SET_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ ACT_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, SET_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FONT,		{ 0 }, (void *)BUTTON_FONT },
	{ GO_TYPE_TEXT_IN_BOX,	{ 0, 0, SET_BUTTON_WIDTH, BUTTON_HEIGHT, FONT_HATTR_CENTER, FONT_VATTR_CENTER }, (void *)"SET" },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_cancel[] = {
	{ GO_TYPE_FORECOLOR,	{ BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FONT,		{ 0 }, (void *)BUTTON_FONT },
	{ GO_TYPE_TEXT_IN_BOX,	{ 0, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT, FONT_HATTR_CENTER, FONT_VATTR_CENTER }, (void *)"CANCEL" },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_cancel_a[] = {
	{ GO_TYPE_FORECOLOR,	{ ACT_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ ACT_BACK_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FONT,		{ 0 }, (void *)BUTTON_FONT },
	{ GO_TYPE_TEXT_IN_BOX,	{ 0, 0, CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT, FONT_HATTR_CENTER, FONT_VATTR_CENTER }, (void *)"CANCEL" },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_back[] = {
	{ GO_TYPE_FORECOLOR,	{ BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/8, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/8,
				  BUTTON_WIDTH/2, BUTTON_HEIGHT/8*7, BUTTON_WIDTH/8, BUTTON_HEIGHT/2} },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/8*3, BUTTON_WIDTH/8*3, BUTTON_HEIGHT/8*2 } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_back_a[] = {
	{ GO_TYPE_FORECOLOR,	{ ACT_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/8, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/8,
				  BUTTON_WIDTH/2, BUTTON_HEIGHT/8*7, BUTTON_WIDTH/8, BUTTON_HEIGHT/2} },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/8*3, BUTTON_WIDTH/8*3, BUTTON_HEIGHT/8*2 } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_0[] = {
	NUM_OBJ("0", BACK_COLOR)
};

static const struct st_graph_object obj_button_0_a[] = {
	NUM_OBJ("0", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_1[] = {
	NUM_OBJ("1", BACK_COLOR)
};

static const struct st_graph_object obj_button_1_a[] = {
	NUM_OBJ("1", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_2[] = {
	NUM_OBJ("2", BACK_COLOR)
};

static const struct st_graph_object obj_button_2_a[] = {
	NUM_OBJ("2", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_3[] = {
	NUM_OBJ("3", BACK_COLOR)
};

static const struct st_graph_object obj_button_3_a[] = {
	NUM_OBJ("3", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_4[] = {
	NUM_OBJ("4", BACK_COLOR)
};

static const struct st_graph_object obj_button_4_a[] = {
	NUM_OBJ("4", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_5[] = {
	NUM_OBJ("5", BACK_COLOR)
};

static const struct st_graph_object obj_button_5_a[] = {
	NUM_OBJ("5", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_6[] = {
	NUM_OBJ("6", BACK_COLOR)
};

static const struct st_graph_object obj_button_6_a[] = {
	NUM_OBJ("6", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_7[] = {
	NUM_OBJ("7", BACK_COLOR)
};

static const struct st_graph_object obj_button_7_a[] = {
	NUM_OBJ("7", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_8[] = {
	NUM_OBJ("8", BACK_COLOR)
};

static const struct st_graph_object obj_button_8_a[] = {
	NUM_OBJ("8", ACT_BACK_COLOR)
};

static const struct st_graph_object obj_button_9[] = {
	NUM_OBJ("9", BACK_COLOR)
};

static const struct st_graph_object obj_button_9_a[] = {
	NUM_OBJ("9", ACT_BACK_COLOR)
};

static const struct st_ui_button_image ui_view_set = {
	{ { BTN_POS_X_SET,  BTN_POS_Y_CANCEL}, {BUTTON_WIDTH * 2, BUTTON_HEIGHT} },
	obj_button_set,
	obj_button_set_a
};

static const struct st_ui_button_image ui_view_cancel = {
	{ {BTN_POS_X_CANCEL,  BTN_POS_Y_CANCEL}, {CANCEL_BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_cancel,
	obj_button_cancel_a
};

static const struct st_ui_button_image ui_view_back = {
	{ {BTN_POS_X_BACK,  BTN_POS_Y_BACK}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_back,
	obj_button_back_a
};

static const struct st_ui_button_image ui_view_0 = {
	{ {BTN_POS_X_0,  BTN_POS_Y_0}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_0,
	obj_button_0_a
};

static const struct st_ui_button_image ui_view_1 = {
	{ {BTN_POS_X_1,  BTN_POS_Y_1}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_1,
	obj_button_1_a
};

static const struct st_ui_button_image ui_view_2 = {
	{ {BTN_POS_X_2,  BTN_POS_Y_2}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_2,
	obj_button_2_a
};

static const struct st_ui_button_image ui_view_3 = {
	{ {BTN_POS_X_3,  BTN_POS_Y_3}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_3,
	obj_button_3_a
};

static const struct st_ui_button_image ui_view_4 = {
	{ {BTN_POS_X_4,  BTN_POS_Y_4}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_4,
	obj_button_4_a
};

static const struct st_ui_button_image ui_view_5 = {
	{ {BTN_POS_X_5,  BTN_POS_Y_5}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_5,
	obj_button_5_a
};

static const struct st_ui_button_image ui_view_6 = {
	{ {BTN_POS_X_6,  BTN_POS_Y_6}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_6,
	obj_button_6_a
};

static const struct st_ui_button_image ui_view_7 = {
	{ {BTN_POS_X_7,  BTN_POS_Y_7}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_7,
	obj_button_7_a
};

static const struct st_ui_button_image ui_view_8 = {
	{ {BTN_POS_X_8,  BTN_POS_Y_8}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_8,
	obj_button_8_a
};

static const struct st_ui_button_image ui_view_9 = {
	{ {BTN_POS_X_9,  BTN_POS_Y_9}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	obj_button_9,
	obj_button_9_a
};

#define BTN_ID_SET	12
#define BTN_ID_CANCEL	11
#define BTN_ID_BACK	10

static struct st_ui_button ui_btn_set = {
	BTN_ID_SET,
	&ui_view_set,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_cancel = {
	BTN_ID_CANCEL,
	&ui_view_cancel,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_back = {
	BTN_ID_BACK,
	&ui_view_back,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_0 = {
	0,
	&ui_view_0,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_1 = {
	1,
	&ui_view_1,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_2 = {
	2,
	&ui_view_2,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_3 = {
	3,
	&ui_view_3,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_4 = {
	4,
	&ui_view_4,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_5 = {
	5,
	&ui_view_5,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_6 = {
	6,
	&ui_view_6,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_7 = {
	7,
	&ui_view_7,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_8 = {
	8,
	&ui_view_8,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button ui_btn_9 = {
	9,
	&ui_view_9,
	UI_BUTTON_ST_NORMAL
};

static struct st_ui_button *ui_tenbutton_view[] = {
	&ui_btn_set,
	&ui_btn_cancel,
	&ui_btn_back,
	&ui_btn_0,
	&ui_btn_1,
	&ui_btn_2,
	&ui_btn_3,
	&ui_btn_4,
	&ui_btn_5,
	&ui_btn_6,
	&ui_btn_7,
	&ui_btn_8,
	&ui_btn_9,
	0
};

static unsigned char str_year[5];
static unsigned char str_month[3];
static unsigned char str_day[3];
static unsigned char str_hour[3];
static unsigned char str_min[3];
static unsigned char str_sec[3];

#define TIMESET_TEXT_WIDTH	16

const struct st_graph_object te_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object te_active_view[] = {
	{ GO_TYPE_FORECOLOR,	{ ACT_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ ACT_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object te_cursor_view[] = {
	{ GO_TYPE_FORECOLOR,	{ CUR_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ CUR_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static struct st_ui_edittext te_year = {
	.view_area	= { {PANEL_X + 8, PANEL_Y + 16}, {(TIMESET_TEXT_WIDTH * 4) + (TEXT_AREA_MARGINE * 2), 32} },
	.normal_view	= te_normal_view,
	.active_view	= te_active_view,
	.cursor_view	= te_cursor_view,
	.text		= str_year,
	.max_text_length = 4,
};

static int month_filter(struct st_ui_edittext *te, unsigned char ch)
{
	if(te->cursor_pos == 0) {
		if((ch == '0') || (ch == '1')) {
			// OK
		} else {
			return 1;
		}
	} else if(te->cursor_pos == 1) {
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
	.view_area	= { {PANEL_X + 8, PANEL_Y + 16 + 52}, {(TIMESET_TEXT_WIDTH * 2) + (TEXT_AREA_MARGINE * 2), 32} },
	.normal_view	= te_normal_view,
	.active_view	= te_active_view,
	.cursor_view	= te_cursor_view,
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

	if(te->cursor_pos == 0) {
		if(ch <= ('0' + (nday/10))) {
			// OK
		} else {
			return 1;
		}
	} else if(te->cursor_pos == 1) {
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
	.view_area	= { {PANEL_X + 8 + 16*4, PANEL_Y + 16 + 52}, {(TIMESET_TEXT_WIDTH * 2) + (TEXT_AREA_MARGINE * 2), 32} },
	.normal_view	= te_normal_view,
	.active_view	= te_active_view,
	.cursor_view	= te_cursor_view,
	.text		= str_day,
	.max_text_length = 2,
	.filter		= day_filter,
};

static int hour_filter(struct st_ui_edittext *te, unsigned char ch)
{
	if(te->cursor_pos == 0) {
		if((ch >= '0') && (ch <= '2')) {
			// OK
		} else {
			return 1;
		}
	} else if(te->cursor_pos == 1) {
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
	.view_area	= { {PANEL_X + 8, PANEL_Y + 16 + 52*2}, {(TIMESET_TEXT_WIDTH * 2) + (TEXT_AREA_MARGINE * 2), 32} },
	.normal_view	= te_normal_view,
	.active_view	= te_active_view,
	.cursor_view	= te_cursor_view,
	.text		= str_hour,
	.max_text_length = 2,
	.filter		= hour_filter,
};

static int minsec_filter(struct st_ui_edittext *te, unsigned char ch)
{
	if(te->cursor_pos == 0) {
		if((ch >= '0') && (ch <= '5')) {
			// OK
		} else {
			return 1;
		}
	}

	return 0;
}

static struct st_ui_edittext te_min = {
	.view_area	= { {PANEL_X + 8 + 50, PANEL_Y + 16 + 52*2}, {(TIMESET_TEXT_WIDTH * 2) + (TEXT_AREA_MARGINE * 2), 32} },
	.normal_view	= te_normal_view,
	.active_view	= te_active_view,
	.cursor_view	= te_cursor_view,
	.text		= str_min,
	.max_text_length = 2,
	.filter		= minsec_filter,
};

static struct st_ui_edittext te_sec = {
	.view_area	= { {PANEL_X + 8 + 100, PANEL_Y + 16 + 52*2}, {(TIMESET_TEXT_WIDTH * 2) + (TEXT_AREA_MARGINE * 2), 32} },
	.normal_view	= te_normal_view,
	.active_view	= te_active_view,
	.cursor_view	= te_cursor_view,
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

static void prepare_te(struct st_ui_edittext *te)
{
	te->flg_active = 0;
	te->cursor_pos = 0;
}

void prepare_timeset(struct st_datetime *datetime)
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

	te_year.flg_active = 1;
}

void draw_timeset(void)
{
	set_forecolor(BACK_COLOR);
	draw_fill_rect(&setting_view_area);

	set_forecolor(FORE_COLOR);
	set_font_by_name(BUTTON_FONT);
	set_graph_obj_scale(1, 1);

	draw_char(PANEL_X + 8 + 16*3, te_month.view_area.pos.y + TEXT_AREA_MARGINE, '/');
	draw_char(PANEL_X + 8 + 16*2+8, te_hour.view_area.pos.y + TEXT_AREA_MARGINE, ':');
	draw_char(PANEL_X + 8 + 16*5+8, te_hour.view_area.pos.y + TEXT_AREA_MARGINE, ':');

	draw_ui_button_list(ui_tenbutton_view);

	draw_ui_edittext_list(settime_textedit);
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

void proc_timeset(struct st_sysevent *event)
{
	struct st_button_event obj_evt;

	if(proc_ui_button_list(&obj_evt, ui_tenbutton_view, event) != 0) {
		switch(obj_evt.id) {
		case BTN_ID_SET:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				set_datetime();
				draw_clock_view();
				disp_mode = MODE_CLOCK;
			}
			break;

		case BTN_ID_CANCEL:
			if(obj_evt.what == UI_BUTTON_EVT_PULL) {
				draw_clock_view();
				disp_mode = MODE_CLOCK;
			}
			break;

		case BTN_ID_BACK:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				move_cursor_ui_edittext_list(settime_textedit, -1);
			}
			break;

		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
		case 8:
		case 9:
			if(obj_evt.what == UI_BUTTON_EVT_PUSH) {
				set_char_ui_edittext_list(settime_textedit, '0' + obj_evt.id);
			}
			break;

		default:
			break;
		}
	}

	if(proc_ui_edittext_list(settime_textedit, event) != 0) {
		// [TODO]
	}
}

/** @file
    @brief	10キー

    @date	2019.09.07
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "graphics.h"
#include "font.h"
#include "graphics_object.h"

#include "ui_style.h"
#include "ui_button.h"

#include "tenkey.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#if GSC_GRAPHICS_DISPLAY_HEIGHT == 480
#define BUTTON_FONT	"num24x48"
#define PANEL_Y		40
#define BUTTON_TOP	0
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 272
#define BUTTON_FONT	"16x24"
#define PANEL_Y		0
#define BUTTON_TOP	0
#elif GSC_GRAPHICS_DISPLAY_HEIGHT == 240
#define BUTTON_FONT	"16x24"
#define PANEL_Y		0
#define BUTTON_TOP	16
#else
#define BUTTON_FONT	"16x24"
#define PANEL_Y		0
#define BUTTON_TOP	24
#endif

#if GSC_GRAPHICS_DISPLAY_WIDTH == 320
#define PANEL_X		160
#define BUTTON_WIDTH	48
#define BUTTON_HEIGHT	48
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 480
#define PANEL_X		256
#define BUTTON_WIDTH	64
#define BUTTON_HEIGHT	64
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 800
#define PANEL_X		488
#define BUTTON_WIDTH	96
#define BUTTON_HEIGHT	96
#else	// 240
#define PANEL_X		0
#define BUTTON_WIDTH	48
#define BUTTON_HEIGHT	48
#endif

#define BUTTON_INTERVAL	4

#define BTN_POS_X_7	(PANEL_X)
#define BTN_POS_Y_7	(PANEL_Y + BUTTON_TOP)

#define BTN_POS_X_8	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_8	(PANEL_Y + BUTTON_TOP)

#define BTN_POS_X_9	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_9	(PANEL_Y + BUTTON_TOP)

#define BTN_POS_X_4	(PANEL_X)
#define BTN_POS_Y_4	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_5	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_5	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_6	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_6	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_1	(PANEL_X)
#define BTN_POS_Y_1	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_2	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_2	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_3	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_3	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_SET	(PANEL_X + 4)
#define BTN_POS_Y_SET	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_0	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_0	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_FORWARD	(PANEL_X + BUTTON_WIDTH + BUTTON_INTERVAL + BUTTON_WIDTH + BUTTON_INTERVAL)
#define BTN_POS_Y_FORWARD	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)

#define BTN_POS_X_BACK	(PANEL_X)
#define BTN_POS_Y_BACK	(PANEL_Y + BUTTON_TOP + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL + BUTTON_HEIGHT + BUTTON_INTERVAL)


static struct st_rect tenkey_view_area = {
	PANEL_X, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT
};


static const struct st_graph_object back_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/8, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/8,
				  BUTTON_WIDTH/2, BUTTON_HEIGHT/8*7, BUTTON_WIDTH/8, BUTTON_HEIGHT/2} },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/8*3, BUTTON_WIDTH/8*3, BUTTON_HEIGHT/8*2 } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_back[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)back_mark},
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_back_a[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)back_mark},
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_ui_button_image ui_view_back = {
	.normal_view	= obj_button_back,
	.select_view	= obj_button_back_a
};

static struct st_ui_button ui_btn_back = {
	.id	= BTN_ID_BACK,
	.view_area	= { {BTN_POS_X_BACK,  BTN_POS_Y_BACK}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.view	= &ui_view_back,
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_backspace = {
	.id	= BTN_ID_BACKSPACE,
	.view_area	= { {BTN_POS_X_BACK,  BTN_POS_Y_BACK}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
//	.font_name	= BUTTON_FONT,
	.name	= "BS",
	.status = UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_delete = {
	.id	= BTN_ID_DELETE,
	.view_area	= { {BTN_POS_X_BACK,  BTN_POS_Y_BACK}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
//	.font_name	= BUTTON_FONT,
	.name	= "DEL",
	.status = UI_BUTTON_ST_NORMAL,
};


static const struct st_graph_object forward_mark[] = {
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_FORE_COLOR } },
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/8*7, BUTTON_HEIGHT/2, BUTTON_WIDTH/2, BUTTON_HEIGHT/8,
				  BUTTON_WIDTH/2, BUTTON_HEIGHT/8*7, BUTTON_WIDTH/8*7, BUTTON_HEIGHT/2} },
	{ GO_TYPE_FILL_BOX,	{ BUTTON_WIDTH/8, BUTTON_HEIGHT/8*3, BUTTON_WIDTH/8*3, BUTTON_HEIGHT/8*2 } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_forward[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_NORMAL_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)forward_mark},
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object obj_button_forward_a[] = {
	{ GO_TYPE_FORECOLOR,	{ UI_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_OBJECT,	{ 0 },  (void *)forward_mark},
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_ui_button_image ui_view_forward = {
	.normal_view	= obj_button_forward,
	.select_view	= obj_button_forward_a
};

static struct st_ui_button ui_btn_forward = {
	.id	= BTN_ID_FORWARD,
	.view_area	= { {BTN_POS_X_FORWARD,  BTN_POS_Y_FORWARD}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.view	= &ui_view_forward,
	.status	= UI_BUTTON_ST_NORMAL,
};


static struct st_ui_button ui_btn_0 = {
	.id	= BTN_ID_0,
	.view_area	= { {BTN_POS_X_0,  BTN_POS_Y_0}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "0",
	.status = UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_1 = {
	.id	= BTN_ID_1,
	.view_area	= { {BTN_POS_X_1,  BTN_POS_Y_1}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "1",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_2 = {
	.id	= BTN_ID_2,
	.view_area	= { {BTN_POS_X_2,  BTN_POS_Y_2}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "2",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_3 = {
	.id	= BTN_ID_3,
	.view_area	= { {BTN_POS_X_3,  BTN_POS_Y_3}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "3",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_4 = {
	.id	= BTN_ID_4,
	.view_area	= { {BTN_POS_X_4,  BTN_POS_Y_4}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "4",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_5 = {
	.id	= BTN_ID_5,
	.view_area	= { {BTN_POS_X_5,  BTN_POS_Y_5}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "5",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_6 = {
	.id	= BTN_ID_6,
	.view_area	= { {BTN_POS_X_6,  BTN_POS_Y_6}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "6",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_7 = {
	.id	= BTN_ID_7,
	.view_area	= { {BTN_POS_X_7,  BTN_POS_Y_7}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "7",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_8 = {
	.id	= BTN_ID_8,
	.view_area	= { {BTN_POS_X_8,  BTN_POS_Y_8}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "8",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_btn_9 = {
	.id	= BTN_ID_9,
	.view_area	= { {BTN_POS_X_9,  BTN_POS_Y_9}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	.font_name	= BUTTON_FONT,
	.name	= "9",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button *ui_tenbutton_view[] = {
	&ui_btn_back,
	&ui_btn_forward,
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

void set_mode_tenkey(int mode)
{
	switch(mode) {
	case 0:
		ui_tenbutton_view[0] = &ui_btn_back;
		break;

	case 1:
		ui_tenbutton_view[0] = &ui_btn_backspace;
		break;

	case 2:
		ui_tenbutton_view[0] = &ui_btn_delete;
		break;

	default:
		ui_tenbutton_view[0] = &ui_btn_back;
		break;
	}
}

void draw_tenkey(void)
{
	set_forecolor(UI_BACK_COLOR);
	draw_fill_rect(&tenkey_view_area);
	set_graph_obj_scale(1, 1);

	draw_ui_button_list(ui_tenbutton_view);
}

int proc_tenkey(struct st_button_event *obj_evt, struct st_sysevent *event)
{
	int rt = 0;

	rt = proc_ui_button_list(obj_evt, ui_tenbutton_view, event);

	return rt;
}

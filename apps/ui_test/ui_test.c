/*
  UIテスト

  2018.03.21 Takashi SHUDO
 */

#include "sysconfig.h"

#include "storage.h"
#include "graphics.h"
#include "sysevent.h"
#include "font.h"
#include "tprintf.h"
#include "str.h"
#include "task/syscall.h"

#include "ui_switch.h"
#include "ui_progressbar.h"
#include "ui_button.h"
#include "ui_edittext.h"
#include "ui_seekbar.h"
#include "ui_selectlist.h"
#include "ui_statictext.h"
#include "ui_style.h"
#include "dialogbox/timeset/timeset.h"
#include "dialogbox/netset/netset.h"

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define UI_MARGINE		12
#define UI_SWITCH_HEIGHT	32
#define UI_SWITCH_TEXT_WIDTH	96
#define UI_BUTTON_WIDTH		96
#define UI_BUTTON_HEIGHT	64
#define UI_EDITTEXT_WIDTH	144
#define UI_EDITTEXT_HEIGHT	32
#define UI_PROGRESSBAR_WIDTH	256
#define UI_PROGRESSBAR_HEIGHT	32
#define UI_SEEKBAR_SIZE		32
#define UI_V_SEEKBAR_X		328
#define UI_V_SEEKBAR_HEIGHT	200
#define UI_H_SEEKBAR_WIDTH	300
#define UI_LIST_X		380
#define UI_V_SCROLLBAR_WIDTH	32
#define UI_LIST_WIDTH		(GSC_GRAPHICS_DISPLAY_WIDTH - UI_LIST_X - UI_V_SCROLLBAR_WIDTH - UI_MARGINE)
#define UI_LIST_ITEM_HEIGHT	(24+2)
#define UI_LIST_HEIGHT		(((GSC_GRAPHICS_DISPLAY_HEIGHT - UI_MARGINE*2)/UI_LIST_ITEM_HEIGHT)*UI_LIST_ITEM_HEIGHT)
#else
#define UI_MARGINE		8
#define UI_SWITCH_HEIGHT	24
#define UI_SWITCH_TEXT_WIDTH	64
#define UI_BUTTON_WIDTH		64
#define UI_BUTTON_HEIGHT	48
#define UI_EDITTEXT_WIDTH	96
#define UI_EDITTEXT_HEIGHT	24
#define UI_PROGRESSBAR_WIDTH	192
#define UI_PROGRESSBAR_HEIGHT	24
#define UI_SEEKBAR_SIZE		24
#define UI_V_SEEKBAR_X		240
#define UI_V_SEEKBAR_HEIGHT	150
#define UI_H_SEEKBAR_WIDTH	200
#define UI_LIST_X		300
#define UI_V_SCROLLBAR_WIDTH	24
#define UI_LIST_WIDTH		(GSC_GRAPHICS_DISPLAY_WIDTH - UI_LIST_X - UI_V_SCROLLBAR_WIDTH - UI_MARGINE)
#define UI_LIST_ITEM_HEIGHT	(16+2)
#define UI_LIST_HEIGHT		(((GSC_GRAPHICS_DISPLAY_HEIGHT - UI_MARGINE*2)/UI_LIST_ITEM_HEIGHT)*UI_LIST_ITEM_HEIGHT)
#endif

static struct st_ui_switch uisw_test1 = {
	.id	= 1,
	.text_area = {
		.pos.x	= UI_MARGINE,
		.pos.y	= UI_MARGINE,
		.sur.width	= UI_SWITCH_TEXT_WIDTH,
		.sur.height	= UI_SWITCH_HEIGHT,
	},
	.font_name	= GSC_FONTS_DEFAULT_FONT,
	.name		= (unsigned char *)"Switch1",
	.status	= UI_SWITCH_ST_NORMAL,
	.value	= 0,
};

static struct st_ui_switch uisw_test2 = {
	.id	= 2,
	.text_area = {
		.pos.x	= UI_MARGINE,
		.pos.y	= UI_MARGINE + UI_SWITCH_HEIGHT + UI_MARGINE,
		.sur.width	= UI_SWITCH_TEXT_WIDTH,
		.sur.height	= UI_SWITCH_HEIGHT,
	},
	.font_name	= GSC_FONTS_DEFAULT_FONT,
	.name		= (unsigned char *)"Switch2",
	.status	= UI_SWITCH_ST_NORMAL,
	.value	= 0,
};

struct st_ui_switch *uisw_test_list[] = {
	&uisw_test1,
	&uisw_test2,
	0
};

#ifdef UICUSTOM
static const struct st_graph_object normal_color_view[] = {
	{ GO_TYPE_FORECOLOR,	{ RGB(240, 240, 240) } },
	{ GO_TYPE_BACKCOLOR,	{ RGB(0, 0, 0) } },
	{ 0, { 0, 0, 0, 0 }}
};
#endif


static struct st_ui_button ui_button1 = {
	.id	= 1,
	.view_area	= {
		.pos.x	= UI_MARGINE,
		.pos.y	= UI_MARGINE + UI_SWITCH_HEIGHT + UI_MARGINE*2 + UI_SWITCH_HEIGHT + UI_MARGINE,
		.sur.width	= UI_BUTTON_WIDTH,
		.sur.height	= UI_BUTTON_HEIGHT,
	},
	.name	= "Button1",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button ui_button2 = {
	.id	= 2,
	.view_area	= {
		.pos.x	= UI_MARGINE + UI_MARGINE + UI_BUTTON_WIDTH,
		.pos.y	= UI_MARGINE + UI_SWITCH_HEIGHT + UI_MARGINE*2 + UI_SWITCH_HEIGHT + UI_MARGINE,
		.sur.width	= UI_BUTTON_WIDTH,
		.sur.height	= UI_BUTTON_HEIGHT,
	},
	.name	= "Button2",
	.status	= UI_BUTTON_ST_NORMAL,
};

static struct st_ui_button *ui_button[] = {
	&ui_button1,
	&ui_button2,
	0
};


static struct st_ui_progressbar progressbar = {
	.view_area.pos.x = UI_MARGINE,
	.view_area.pos.y = UI_MARGINE + UI_SWITCH_HEIGHT + UI_MARGINE*2 + UI_SWITCH_HEIGHT + UI_MARGINE
	+ UI_BUTTON_HEIGHT + UI_MARGINE,
	.view_area.sur.width = UI_PROGRESSBAR_WIDTH,
	.view_area.sur.height = UI_PROGRESSBAR_HEIGHT,
#ifdef UICUSTOM
	.view = normal_color_view,
#endif
	.max_value = 256,
	.value = 100,
};


static uchar test_str1[10] = "Edittext1";
static struct st_ui_edittext te_test1 = {
	.view_area.pos.x	= UI_MARGINE + UI_SWITCH_TEXT_WIDTH + UI_SWITCH_HEIGHT * 2,
	.view_area.pos.y	= UI_MARGINE,
	.view_area.sur.width	= UI_EDITTEXT_WIDTH,
	.view_area.sur.height	= UI_EDITTEXT_HEIGHT,
	.text_area_margin	= 4,
	.font_name		= GSC_FONTS_DEFAULT_FONT,
	.flg_uneditable		= 1,
	.flg_active		= 0,
	.text			= test_str1,
	.max_text_length	= 9,
};

static uchar test_str2[10] = "Edittext2";
static struct st_ui_edittext te_test2 = {
	.view_area.pos.x	= UI_MARGINE + UI_SWITCH_TEXT_WIDTH + UI_SWITCH_HEIGHT * 2,
	.view_area.pos.y	= UI_MARGINE + UI_SWITCH_HEIGHT + UI_MARGINE,
//	.view_area.pos.y	= UI_MARGINE + EDITTEXT_HEIGHT + UI_MARGINE - 2,
	.view_area.sur.width	= UI_EDITTEXT_WIDTH,
	.view_area.sur.height	= UI_EDITTEXT_HEIGHT,
	.text_area_margin	= 4,
	.font_name		= GSC_FONTS_DEFAULT_FONT,
	.flg_uneditable		= 1,
	.flg_active		= 0,
	.text			= test_str2,
	.max_text_length	= 9,
};

static struct st_ui_edittext *te_test[] = {
	&te_test1,
	&te_test2,
	0
};

static struct st_ui_seekbar sb_vtest = {
	.view_area = {
		.pos.x = UI_V_SEEKBAR_X,
		.pos.y = UI_MARGINE,
		.sur.width = UI_SEEKBAR_SIZE,
		.sur.height = UI_V_SEEKBAR_HEIGHT,
	},
	.type = UI_SKB_TYPE_VERTICAL,
	.attr = UI_SKB_ATTR_REALTIME_VALUE_CAHNGE,
	.flg_active = 1,
#ifdef UICUSTOM
	.normal_view = normal_color_view,
	.bar_color = active_bar_color,
	.bar_inactive_color = inactive_bar_color,
#endif
	.value = 50,
	.max_value = 99,
};

static struct st_ui_seekbar sb_htest = {
	.view_area = {
		.pos.x = UI_MARGINE,
		.pos.y = UI_MARGINE + UI_SWITCH_HEIGHT + UI_MARGINE*2 + UI_SWITCH_HEIGHT + UI_MARGINE
		+ UI_BUTTON_HEIGHT + UI_MARGINE + UI_PROGRESSBAR_HEIGHT + UI_MARGINE,
		.sur.width = UI_H_SEEKBAR_WIDTH,
		.sur.height = UI_SEEKBAR_SIZE,
	},
	.type = UI_SKB_TYPE_HOLIZONTAL,
	.attr = UI_SKB_ATTR_REALTIME_VALUE_CAHNGE,
	.flg_active = 1,
#ifdef UICUSTOM
	.normal_view = normal_color_view,
	.bar_color = active_bar_color,
	.bar_inactive_color = inactive_bar_color,
#endif
	.value = 50,
	.max_value = 99,
};


extern struct st_ui_selectlist test_select;

static struct st_ui_scrollbar sl_scrollbar = {
	.view_area = {
		.pos.x = UI_LIST_X + UI_LIST_WIDTH,
		.pos.y = UI_MARGINE,
		.sur.width = UI_V_SCROLLBAR_WIDTH,
		.sur.height = UI_LIST_HEIGHT,
	},
#ifdef UICUSTOM
	.normal_view = sb_normal_view,
	.select_view = sb_select_view,
#endif
	.status = UI_SCB_ST_STILL,
	.selectlist = &test_select,
};

static void draw_list_item(struct st_ui_selectlist *selectlist, struct st_box *box, int item_num, int flg_select)
{
	uchar str[32];

	//tprintf("X=%d, Y=%d, W=%d, H=%d\n", box->pos.x, box->pos.y, box->sur.width, box->sur.height);
	tsnprintf((char *)str, 32, "List Item %d", item_num);
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(box);
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_str_in_box(box, FONT_HATTR_LEFT, FONT_VATTR_CENTER, str);
}

struct st_ui_selectlist test_select = {
	.view_area = {
		.pos.x	= UI_LIST_X,
		.pos.y	= UI_MARGINE,
		.sur.width	= UI_LIST_WIDTH,
		.sur.height	= UI_LIST_HEIGHT,
	},
#ifdef UICUSTOM
	.normal_view = album_list_view,
#endif
	.item_height = UI_LIST_ITEM_HEIGHT,
	.item_count = 0,
	.top_item_num = 0,
	.select_item_num = 0,
	.item_draw_func = draw_list_item,
	.scrollbar = &sl_scrollbar,
};

static unsigned int fore_color = RGB(255,255,255);
static unsigned int back_color = RGB(20,20,20);

static struct st_ui_statictext stext_test = {
	.view_area = {
		.pos.x		= 16,
		.pos.y		= 300,
		.sur.width	= 16 * 10,
		.sur.height	= 28,
	},
	.fore_color	= &fore_color,
	.back_color	= &back_color,
#ifdef GSC_FONTS_ENABLE_FONT_JISKAN16
	.font_name	= "jiskan16",
#else
	.font_name	= "16x24",
#endif
	.hattr		= FONT_HATTR_LEFT,
//	.hattr		= FONT_HATTR_CENTER,
//	.hattr		= FONT_HATTR_RIGHT,
//	.vattr		= FONT_VATTR_TOP,
	.vattr		= FONT_VATTR_CENTER,
//	.vattr		= FONT_VATTR_BOTTOM,
	.fillattr	= UI_STATICTEXT_FILLARRT_FILL,
#ifdef GSC_FONTS_ENABLE_FONT_JISKAN16
	.text		= (uchar *)"漢字表示テスト",
#else
	.text		= (uchar *)"Static Text",
#endif
};


static void draw_ui_test(void)
{
	set_backcolor(UI_BACK_COLOR);
	fill_screen();
	draw_ui_switch_list(uisw_test_list);
	draw_ui_progressbar(&progressbar);
	draw_ui_button_list(ui_button);
	draw_ui_edittext_list(te_test);
	draw_ui_seekbar(&sb_vtest);
	draw_ui_seekbar(&sb_htest);
	draw_ui_selectlist(&test_select);
	draw_ui_statictext(&stext_test);
}

static int pb_val = 0;
static int flg_pb_down = 0;
static struct st_ui_edittext *last_te;


int ui_task(void *arg)
{
	struct st_sysevent event;

	task_sleep(100);

	load_network_setting();
	set_param_ui_selectlist(&test_select, 32, 0, 0);
	draw_ui_test();

	while(1) {
		if(get_event(&event, 50)) {
			struct st_switch_event ui_switch_event;
			struct st_button_event ui_button_event;
			int btnevt = 0;
			int sb_val;
			struct st_ui_selectlist_event sl_evt;

			if(proc_ui_switch_list(&ui_switch_event, uisw_test_list, &event) != 0) {
				tprintf("id = %d, event = %d\n", ui_switch_event.id, ui_switch_event.what);
				switch(ui_switch_event.id) {
				case 1:
					switch(ui_switch_event.what) {
					case UI_SWITCH_EVT_ON:
						editable_ui_edittext(&te_test1);
						break;
					case UI_SWITCH_EVT_OFF:
						uneditable_ui_edittext(&te_test1);
						break;
					default:
						break;
					}
					break;

				case 2:
					switch(ui_switch_event.what) {
					case UI_SWITCH_EVT_ON:
						editable_ui_edittext(&te_test2);
						break;
					case UI_SWITCH_EVT_OFF:
						uneditable_ui_edittext(&te_test2);
						break;
					default:
						break;
					}
					break;

				default:
					break;
				}
			}

			btnevt = proc_ui_button_list(&ui_button_event, ui_button, &event);
			if(btnevt != 0) {
				tprintf("button id = %d, event = %d\n", ui_button_event.id, ui_button_event.what);

				switch(ui_button_event.id) {
				case 1:
					if(ui_button_event.what == UI_BUTTON_EVT_PULL) {
						static struct st_systime now_time = { 0 };
						struct st_datetime datetime = { 0 };

						get_systime(&now_time);
						systime_to_datetime(&datetime, &now_time);
						prepare_timeset(&datetime);
						open_timeset_dialog(0);
						draw_ui_test();
					}
					break;

				case 2:
					if(ui_button_event.what == UI_BUTTON_EVT_PULL) {
						int rt = 0;

						rt = open_netset_dialog(0);
						if(rt == 1) {
							// Nework setting
							tprintf("Network new setting\n");
						} else {
							tprintf("Network setting canceled\n");
						}
						draw_ui_test();
					}
					break;

				default:
					break;
				}
			}


			proc_ui_edittext_list(te_test, &event, &last_te);
			proc_ui_seekbar(&sb_vtest, &event, &sb_val);
			proc_ui_seekbar(&sb_htest, &event, &sb_val);

			if(proc_ui_selectlist(&sl_evt, &test_select, &event) != 0) {
			}
		}

		if(flg_pb_down == 0) {
			pb_val ++;
			if(pb_val > progressbar.max_value) {
				flg_pb_down = 1;
			}
		} else {
			pb_val --;
			if(pb_val <= 0) {
				flg_pb_down = 0;
			}
		}
		set_value_ui_progressbar(&progressbar, pb_val);
	}

	return 0;
}

#define SIZEOFAPPTS	(1024*8)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)] ATTR_STACK;

void startup_ui_test(void)
{
	mount_storage(1, "qspi", FSNAME_GSFFS);

	task_exec(ui_task, "ui_test", 2, &tcb,
		     stack, SIZEOFAPPTS, 0);
}

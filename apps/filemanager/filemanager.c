/** @file
    @brief	ファイルマネージャ

    @date	2017.11.15
    @author	Takashi SHUDO
*/

#include "shell.h"
#include "tprintf.h"
#include "storage.h"
#include "file.h"
#include "sysevent.h"
#include "key.h"
#include "console.h"
#include "str.h"
#include "memory.h"
#include "task/syscall.h"

#include "ui_button.h"

#include "filelist_data.h"
#include "filelist_view.h"

#ifdef GSC_DEV_ENABLE_RTC
#include "clock_view.h"
#endif

#ifdef GSC_DEV_ENABLE_AUDIO
#include "volume_view.h"
#include "mini_musicplay_view.h"
#endif

#include "filemanager.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

static const struct st_graph_object fm_info_view[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ FM_INFO_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object fm_normal_view[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_LIST_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ FM_LIST_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object fm_select_view[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_ACTIVE_FORE_COLOR } },
	{ GO_TYPE_BACKCOLOR,	{ FM_ACTIVE_BACK_COLOR } },
	{ 0, { 0, 0, 0, 0 }}
};

static struct st_filelist_view filelist_view = {
	.font_name = DEFAULT_FONT,
	.info_area = {
		.pos.x = 0,
		.pos.y = 0,
		.sur.width = GSC_GRAPHICS_DISPLAY_WIDTH,
		.sur.height = INFO_AREA_HEIGHT,
	},
	.list_area = {
		.pos.x = 0,
		.pos.y = INFO_AREA_HEIGHT,
		.sur.width = GSC_GRAPHICS_DISPLAY_WIDTH - SCRBAR_WIDTH,
		.sur.height = GSC_GRAPHICS_DISPLAY_HEIGHT - INFO_AREA_HEIGHT - CONTROL_HEIGHT,
	},
	.info_view = fm_info_view,
	.list_view = fm_normal_view,
	.path = "0:",
	.selectlist = {
		.view_area = {
			.pos.x = 0,
			.pos.y = INFO_AREA_HEIGHT,
			.sur.width = GSC_GRAPHICS_DISPLAY_WIDTH - SCRBAR_WIDTH,
			.sur.height = GSC_GRAPHICS_DISPLAY_HEIGHT - INFO_AREA_HEIGHT - CONTROL_HEIGHT,
		},
		.normal_view = fm_normal_view,
		.select_view = fm_select_view,
		.item_draw_func = draw_file_item,
	},
	.scrollbar = {
		.view_area = {
			.pos.x = GSC_GRAPHICS_DISPLAY_WIDTH - SCRBAR_WIDTH,
			.pos.y = INFO_AREA_HEIGHT,
			.sur.width = SCRBAR_WIDTH,
			.sur.height = GSC_GRAPHICS_DISPLAY_HEIGHT - INFO_AREA_HEIGHT - CONTROL_HEIGHT,
		},
		.normal_view = fm_normal_view,
		.select_view = fm_select_view,
		.status = UI_SCB_ST_STILL,
	}
};

//static const box dup_btn_box	= { BTN_POS_X_DUP,  BTN_POS_Y_DUP, BUTTON_WIDTH, BUTTON_HEIGHT };
//static const box ddownbtn_box	= { BTN_POS_X_DDOWN,  BTN_POS_Y_DDOWN, BUTTON_WIDTH, BUTTON_HEIGHT };

static const struct st_graph_object dup_btn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_FORE_COLOR } },
/*	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3,
	BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },*/
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/2, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object dup_btn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_FORE_COLOR } },
/*	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/2, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3,
	BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },*/
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/2, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object ddownbtn_obj[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_FORE_COLOR } },
/*	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4,
	BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/4*3} },*/
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object ddownbtn_obj_a[] = {
	{ GO_TYPE_FORECOLOR,	{ FM_ACTIVE_BACK_COLOR } },
	{ GO_TYPE_ROUND_FILL_BOX,	{ 0, 0, BUTTON_WIDTH, BUTTON_HEIGHT,  BUTTON_WIDTH/8 } },
	{ GO_TYPE_FORECOLOR,	{ FM_INFO_FORE_COLOR } },
/*	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4,
	BUTTON_WIDTH/4*3, BUTTON_HEIGHT/4, BUTTON_WIDTH/2, BUTTON_HEIGHT/4*3} },*/
	{ GO_TYPE_VERTEX4,	{ BUTTON_WIDTH/4, BUTTON_HEIGHT/4, BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2,
				  BUTTON_WIDTH/4*3, BUTTON_HEIGHT/2, BUTTON_WIDTH/4, BUTTON_HEIGHT/4*3} },
	{ 0, { 0, 0, 0, 0 }}
};

/* [◀] */
static const struct st_ui_button_image ui_view_dup = {
	dup_btn_obj,
	dup_btn_obj_a
};

static struct st_ui_button ui_btn_dup = {
	UO_ID_UP,
	{ {BTN_POS_X_DUP,  BTN_POS_Y_DUP}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_dup,
	UI_BUTTON_ST_NORMAL
};

/* [▶] */
static const struct st_ui_button_image ui_view_ddown = {
	ddownbtn_obj,
	ddownbtn_obj_a
};

static struct st_ui_button ui_btn_ddown = {
	UO_ID_DOWN,
	{ {BTN_POS_X_DDOWN,  BTN_POS_Y_DDOWN}, {BUTTON_WIDTH, BUTTON_HEIGHT} },
	&ui_view_ddown,
	UI_BUTTON_ST_NORMAL
};

struct st_ui_button *ui_listbtn_view[] = {
	&ui_btn_dup,
	&ui_btn_ddown,
	0
};

void draw_filemanager(void)
{
	struct st_rect frect = { 0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };
	set_forecolor(RGB(0, 0, 0));
	draw_fill_rect(&frect);

	draw_filelist_view(&filelist_view);

	draw_ui_button_list(ui_listbtn_view);

#ifdef GSC_DEV_ENABLE_RTC
	draw_clock();
#endif

#ifdef GSC_DEV_ENABLE_AUDIO
	draw_volume_view();
	draw_mini_musicplay_view();
#endif
}

static void init_filemanager(void)
{
	init_filelist_view(&filelist_view);

	draw_filemanager();

#ifdef GSC_DEV_ENABLE_AUDIO
	init_volume_view();
#endif
}

static char select_file_name[FF_MAX_LFN + 1];

static void do_select_file(void)
{
	get_filelist_select_fname(&filelist_view, select_file_name, FF_MAX_LFN);

	do_file_operation((uchar *)select_file_name, (uchar *)"open");
}

int filemanager_proc(void)
{
	init_filemanager();

#ifdef GSC_DEV_ENABLE_RTC
	startup_clock();
#endif

	while(1) {
		struct st_sysevent event;

		if(get_event(&event, 50)) {
			struct st_button_event obj_evt;
			if(proc_ui_button_list(&obj_evt, ui_listbtn_view, &event) != 0) {
				DTPRINTF(0x01, "ID:%d EVENT:%d\n", obj_evt.id, obj_evt.what);
				switch(obj_evt.id) {
				case UO_ID_UP:
					switch(obj_evt.what) {
					case UI_BUTTON_EVT_PUSH:
						updir_filelist_view(&filelist_view);
						break;

					default:
						break;
					}
					break;

				case UO_ID_DOWN:
					switch(obj_evt.what) {
					case UI_BUTTON_EVT_PUSH:
						downdir_filelist_view(&filelist_view);
						break;

					default:
						break;
					}
					break;

					break;

				default:
					break;
				}
			}

			int evt = filelist_view_proc(&filelist_view, &event);
			if(evt != FLV_EVT_NONE) {
				if(evt == FLV_EVT_FILE_SECCOND_SEL) {
					do_select_file();
				}
			}
			//draw_filelist_view(&filelist_view);
			//draw_ui_button_list(ui_listbtn_view);

#ifdef GSC_DEV_ENABLE_AUDIO
			volume_proc(&event);
			mini_musicplay_proc(&event);
#endif
		}
	}

	return 0;
}

static int filemanager_task(void *arg)
{
	task_sleep(100);

	filemanager_proc();

	return 0;
}

static struct st_tcb tcb;
#define SIZEOFSTACK	(1024*8)
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)] ATTR_STACK;
extern const struct st_file_operation * const file_operation[];

void startup_filemanager(void)
{
	set_file_operation(file_operation);

	task_exec(filemanager_task, "filemanager", TASK_PRIORITY_APP_HIGH, &tcb,
		  stack, SIZEOFSTACK, 0);
}

/** @file
    @brief	時計アプリケーション

    @date	2017.05.27
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "graphics.h"
#include "graphics_object.h"
#include "sysevent.h"
#include "key.h"
#include "font.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "random.h"
#include "datetime.h"
#include "task/syscall.h"
#include "sysconfig.h"

#include "clock.h"
#include "timeset.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#if GSC_GRAPHICS_DISPLAY_WIDTH == 320
#define GO_GRID	40
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 480
#define GO_GRID	64
#elif GSC_GRAPHICS_DISPLAY_WIDTH == 800
#define GO_GRID	112
#else
#define GO_GRID	40
#endif

#define FONT_W	(GO_GRID * 7)
#define FONT_H	(GO_GRID * 12)

static const struct st_graph_object go_num_0[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, 0, GO_GRID*2, GO_GRID, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID, 0 } },
	{ GO_TYPE_FILL_BOX,	{ 0, GO_GRID*2, GO_GRID, GO_GRID*7, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID, GO_GRID*7, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*9, GO_GRID*2, GO_GRID, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*9, GO_GRID*2, GO_GRID, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*2, GO_GRID, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_1[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_VERTEX4,	{ GO_GRID*3, GO_GRID*0,
				  GO_GRID*2, GO_GRID*1,
				  GO_GRID*2, GO_GRID*2,
				  GO_GRID*3, GO_GRID*2 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*3, GO_GRID*0, GO_GRID*1, GO_GRID*10, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*3, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_2[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*0, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*2, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID*1, GO_GRID*1+GO_GRID/4, } },
	{ GO_TYPE_VERTEX4,	{ GO_GRID*5, GO_GRID*3,
				  GO_GRID*0, GO_GRID*10,
				  GO_GRID*1+GO_GRID/4, GO_GRID*10,
				  GO_GRID*6, GO_GRID*3+GO_GRID/4 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*10, GO_GRID*6, GO_GRID, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_3[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*0, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*2, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*4, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*5, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*7, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*8, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*7, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*9, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*9, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*2, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_4[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*0, GO_GRID*1, GO_GRID*4, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*0, GO_GRID*1, GO_GRID*11, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*4, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*5, GO_GRID*3, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_5[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*0, GO_GRID*6, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*1, GO_GRID*1, GO_GRID*4, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*5, GO_GRID*4, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*7, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*7, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*9, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*9, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*8, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*2, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_6[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*0, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*2, GO_GRID*1, GO_GRID*7, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*1, GO_GRID*5, GO_GRID*3, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*7, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*7, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*9, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*9, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*2, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_7[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*2, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*0, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID*1, GO_GRID*9, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_8[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*0, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*2, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*4, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*4, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*5, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*7, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*7, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*7, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*7, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*9, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*9, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*2, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_num_9[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*2, GO_GRID*2, GO_GRID*1, 1 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*0, GO_GRID*2, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*2, GO_GRID*2, GO_GRID*1, 0 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*2, GO_GRID*1, GO_GRID*2, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*5, GO_GRID*2, GO_GRID*1, GO_GRID*7, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*4, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*5, GO_GRID*3, GO_GRID*1, } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*0, GO_GRID*8, GO_GRID*1, GO_GRID*1, } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*2, GO_GRID*9, GO_GRID*2, GO_GRID*1, 2 } },
	{ GO_TYPE_SECTOR,	{ GO_GRID*4, GO_GRID*9, GO_GRID*2, GO_GRID*1, 3 } },
	{ GO_TYPE_FILL_BOX,	{ GO_GRID*2, GO_GRID*10, GO_GRID*2, GO_GRID*1, } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_slash[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_VERTEX4,	{ GO_GRID*5, GO_GRID*0,
				  GO_GRID*0, GO_GRID*10+GO_GRID/2,
				  GO_GRID*1, GO_GRID*11,
				  GO_GRID*6, GO_GRID*0+GO_GRID/2 } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object go_colon[] = {
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE } },
	{ GO_TYPE_FILL_BOX,	{ 0, 0, FONT_W, FONT_H } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL } },
	{ GO_TYPE_FILL_CIRCLE,	{ GO_GRID*3, GO_GRID*3, GO_GRID*1 } },
	{ GO_TYPE_FILL_CIRCLE,	{ GO_GRID*3, GO_GRID*8, GO_GRID*1 } },
	{ 0, { 0, 0, 0, 0 }}
};

static const struct st_graph_object *go_num[12] = {
	go_slash,
	go_num_0,
	go_num_1,
	go_num_2,
	go_num_3,
	go_num_4,
	go_num_5,
	go_num_6,
	go_num_7,
	go_num_8,
	go_num_9,
	go_colon
};

#if 0
#define SC_NUME	12
#define SC_DENO	18
#else
#define SC_NUME	2
#define SC_DENO	16
#endif

static struct st_datetime now_datetime;

static int sc_nume = SC_NUME;
static int sc_deno = SC_DENO;
static int frame_num;
static int disp_frame = 0;
static unsigned char l_date_str[32];
static unsigned char date_str[32];
static unsigned char l_time_str[32];
static unsigned char time_str[32];

int disp_mode = MODE_CLOCK;

static void draw_num_char(short x, short y, unsigned char ch)
{
	if((ch >= '/') && (ch <= ':')) {
		draw_graph_object(x, y, go_num[ch - '/']);
	}
}

static void set_font_scale(int nume, int deno)
{
	sc_nume = nume;
	sc_deno = deno;

	set_graph_obj_scale(sc_nume, sc_deno);
}

static void draw_num_str(short x, short y, unsigned char *str, unsigned char *oldstr)
{
	int i = x;

	while(*str) {
		if(frame_num >= 2) {
			draw_num_char(i, y, *str);
		} else {
			if(*str != *oldstr) {
				draw_num_char(i, y, *str);
				*oldstr = *str;
			}
		}
		str ++;
		oldstr ++;
		i += ((int)FONT_W * sc_nume)/sc_deno;
	}
}

static struct st_rect clock_view_area = {
	0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT
};

static struct st_systime now_time;

void draw_clock_view(void)
{
	set_forecolor(BACK_COLOR);
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_fill_rect(&clock_view_area);

	get_systime(&now_time);
	systime_to_datetime(&now_datetime, &now_time);
	draw_clock(1);
}

void draw_clock(int flg_update)
{
	int draw_frame;

	if(flg_update != 0) {
		memoryset(l_date_str, 0, 32);
		memoryset(l_time_str, 0, 32);
	}

	if(frame_num >= 2) {
		if(disp_frame == 0) {
			draw_frame = 1;
		} else {
			draw_frame = 0;
		}
		set_draw_frame(draw_frame);
	}

	if(flg_update != 0) {
		set_forecolor(BACK_COLOR);
		set_draw_mode(GRP_DRAWMODE_NORMAL);
		draw_fill_rect(&clock_view_area);
	}

	set_forecolor(FORE_COLOR);
	set_backcolor(BACK_COLOR);

	date_to_str((char *)date_str, &now_datetime);
	DTPRINTF(0x01, "%s\n", date_str);
	set_font_scale(SC_NUME, SC_DENO*2);
	draw_num_str(20, 20, date_str, l_date_str);

	time_to_str((char *)time_str, &now_datetime);
	DTPRINTF(0x01, "%s\n", time_str);
	set_font_scale(SC_NUME, SC_DENO);
	draw_num_str(20, 20+((FONT_H*SC_NUME)/SC_DENO), time_str, l_time_str);

	if(frame_num >= 2) {
		if(disp_frame == 0) {
			disp_frame = 1;
		} else {
			disp_frame = 0;
		}
		set_display_frame(disp_frame);
	}
}

struct st_tcb clock_tcb;

#define EVT_SECCOUNTUP	30000

static void sec_timer(void *sp, unsigned long long systime)
{
	struct st_sysevent event;

	//tkprintf("systime= %lld\n", systime);
	event.what = EVT_SECCOUNTUP;
	push_event_interrupt(sp, &event);
}

static void proc_clock(struct st_sysevent *event)
{
	switch(event->what) {
	case EVT_TOUCHSTART:
		prepare_timeset(&now_datetime);
		if(frame_num >= 2) {
			set_draw_frame(disp_frame);
		}
		draw_timeset();
		disp_mode = MODE_SETTING;
		break;

	case EVT_SECCOUNTUP:
		//tkprintf("disp_frame = %d\n", disp_frame);
		get_systime(&now_time);
		systime_to_datetime(&now_datetime, &now_time);
		draw_clock(0);
		break;

	default:
		break;
	}
}

static int clock_task(char *arg)
{
	int i;

	task_sleep(100);
	frame_num = get_frame_num();
	set_forecolor(BACK_COLOR);
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	if(frame_num >= 2) {
		for(i=0; i<frame_num; i++) {
			set_draw_frame(i);
			draw_fill_rect(&clock_view_area);
		}
		set_draw_frame(0);
	}

	register_sec_timer_func(sec_timer);

	tprintf("Start Clock task\n");

	get_systime(&now_time);
	systime_to_datetime(&now_datetime, &now_time);
	draw_clock(1);

	while(1) {
		struct st_sysevent event;

		if(get_event(&event, 50)) {
			switch(disp_mode) {
			case MODE_CLOCK:
				proc_clock(&event);
				break;

			case MODE_SETTING:
				proc_timeset(&event);
				break;

			default:
				break;
			}
		}
	}

	return 0;
}

#define SIZEOFAPPTS	(1024*8)
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)];

void startup_clock(void)
{
	task_exec(clock_task, "clock", TASK_PRIORITY_APP_HIGH, &clock_tcb,
		  stack, SIZEOFAPPTS, 0);
}

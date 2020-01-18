/** @file
    @brief	グラフィックス集合体テスト

    @date	2019.12.01
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "graphics.h"
#include "sysevent.h"
#include "key.h"
#include "font.h"
#include "tprintf.h"
#include "random.h"
#include "str.h"
#include "shell.h"
#include "task/syscall.h"
#include "graphics.h"
#include "graphics_object.h"

//#define SCALE_96
#define SCALE_64
//#define SCALE_32

#ifdef SCALE_96
#define GOBJ_WIDTH	96
#define GOBJ_WWIDTH	144
#define GOBJ_HEIGHT	96
#endif
#ifdef SCALE_64
#define GOBJ_WIDTH	64
#define GOBJ_WWIDTH	96
#define GOBJ_HEIGHT	64
#endif
#ifdef SCALE_32
#define GOBJ_WIDTH	32
#define GOBJ_WWIDTH	48
#define GOBJ_HEIGHT	32
#endif

#define GR	32
#define GRW	48
#define GX(x)	(GOBJ_WIDTH/GR*x)
#define GXW(x)	(GOBJ_WWIDTH/GRW*x)
#define GY(y)	(GOBJ_HEIGHT/GR*y)

const struct st_graph_object gobj_radio[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(4), GY(12), GX(24), GY(16) } },
	{ GO_TYPE_VERTEX4,	{ GX(14), GY(2), GX(15), GY(3), GX(6), GY(12), GX(4), GY(12) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(20), GY(20), GX(6) } },
	{ GO_TYPE_FILL_BOX,	{ GX(6), GY(14), GX(6), GY(12) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_cd[] = {
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(11) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(4) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_cdlist[] = {
	{ GO_TYPE_FILL_CIRCLE,	{ GX(7), GY(10), GX(2) } },
	{ GO_TYPE_FILL_BOX,	{ GX(11), GY(8), GX(16), GY(4) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(7), GY(16), GX(2) } },
	{ GO_TYPE_FILL_BOX,	{ GX(11), GY(14), GX(16), GY(4) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(7), GY(22), GX(2) } },
	{ GO_TYPE_FILL_BOX,	{ GX(11), GY(20), GX(16), GY(4) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_playpause[] = {
	{ GO_TYPE_TRIANGLE,	{ GXW(6), GY(10), GXW(16), GY(16), GXW(6), GY(22) } },
	{ GO_TYPE_VERTEX4,	{ GXW(28), GY(7), GXW(30), GY(8), GXW(20), GY(24), GXW(18), GY(23) } },
	{ GO_TYPE_FILL_BOX,	{ GXW(32), GY(10), GXW(4), GY(12) } },
	{ GO_TYPE_FILL_BOX,	{ GXW(40), GY(10), GXW(4), GY(12) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_ff[] = {
	{ GO_TYPE_TRIANGLE,	{ GX(4), GY(10), GX(14), GY(16), GX(4), GY(22) } },
	{ GO_TYPE_TRIANGLE,	{ GX(14), GY(10), GX(24), GY(16), GX(14), GY(22) } },
	{ GO_TYPE_FILL_BOX,	{ GX(24), GY(10), GX(4), GY(12) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_fr[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(4), GY(10), GX(4), GY(12) } },
	{ GO_TYPE_TRIANGLE,	{ GX(18), GY(10), GX(18), GY(22), GX(8), GY(16) } },
	{ GO_TYPE_TRIANGLE,	{ GX(28), GY(10), GX(28), GY(22), GX(18), GY(16) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_repeat[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(4), GY(14), GX(2), GY(2) } },
	{ GO_TYPE_SECTOR,	{ GX(8), GY(14), GX(4), GY(2), 1 } },
	{ GO_TYPE_FILL_BOX,	{ GX(8), GY(10), GX(15), GY(2) } },
	{ GO_TYPE_TRIANGLE,	{ GX(23), GY(8), GX(26), GY(11), GX(23), GY(14) } },
	{ GO_TYPE_TRIANGLE,	{ GX(6), GY(21), GX(9), GY(18), GX(9), GY(24) } },
	{ GO_TYPE_FILL_BOX,	{ GX(9), GY(20), GX(15), GY(2) } },
	{ GO_TYPE_SECTOR,	{ GX(24), GY(18), GX(4), GY(2), 3 } },
	{ GO_TYPE_FILL_BOX,	{ GX(26), GY(16), GX(2), GY(2) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_shuffle[] = {
	{ GO_TYPE_VERTEX4,	{ GX(4), GY(10), GX(11), GY(10), GX(10), GY(12), GX(4), GY(12) } },
	{ GO_TYPE_VERTEX4,	{ GX(11), GY(10), GX(21), GY(20), GX(20), GY(22), GX(10), GY(12) } },
	{ GO_TYPE_VERTEX4,	{ GX(21), GY(20), GX(26), GY(20), GX(26), GY(22), GX(20), GY(22) } },
	{ GO_TYPE_TRIANGLE,	{ GX(26), GY(18), GX(29), GY(21), GX(26), GY(24) } },
	{ GO_TYPE_VERTEX4,	{ GX(4), GY(20), GX(10), GY(20), GX(11), GY(22), GX(4), GY(22) } },
	{ GO_TYPE_VERTEX4,	{ GX(13), GY(17), GX(15), GY(18), GX(11), GY(22), GX(10), GY(20) } },
	{ GO_TYPE_VERTEX4,	{ GX(16), GY(14), GX(20), GY(10), GX(21), GY(12), GX(18), GY(15) } },
	{ GO_TYPE_VERTEX4,	{ GX(20), GY(10), GX(26), GY(10), GX(26), GY(12), GX(21), GY(12) } },
	{ GO_TYPE_TRIANGLE,	{ GX(26), GY(8), GX(29), GY(11), GX(26), GY(14) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_shuffle24[] = {
	{ GO_TYPE_VERTEX4,	{ GX(8), GY(7), GX(16), GY(7), GX(15), GY(9), GX(8), GY(9) } },
	{ GO_TYPE_VERTEX4,	{ GX(16), GY(7), GX(32), GY(23), GX(31), GY(25), GX(15), GY(9) } },
	{ GO_TYPE_VERTEX4,	{ GX(32), GY(23), GX(38), GY(23), GX(38), GY(25), GX(31), GY(25) } },
	{ GO_TYPE_TRIANGLE,	{ GX(38), GY(21), GX(41), GY(24), GX(38), GY(27) } },
	{ GO_TYPE_VERTEX4,	{ GX(8), GY(23), GX(15), GY(23), GX(16), GY(25), GX(8), GY(25) } },
	{ GO_TYPE_VERTEX4,	{ GX(15), GY(23), GX(21), GY(17), GX(23), GY(18), GX(16), GY(25) } },
	{ GO_TYPE_VERTEX4,	{ GX(24), GY(14), GX(31), GY(7), GX(32), GY(9), GX(26), GY(15) } },
	{ GO_TYPE_VERTEX4,	{ GX(31), GY(7), GX(38), GY(7), GX(38), GY(9), GX(32), GY(9) } },
	{ GO_TYPE_TRIANGLE,	{ GX(38), GY(5), GX(41), GY(8), GX(38), GY(11) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_up[] = {
	{ GO_TYPE_TRIANGLE,	{ GX(16), GY(10), GX(22), GY(22), GX(10), GY(22) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_down[] = {
	{ GO_TYPE_TRIANGLE,	{ GX(10), GY(10), GX(22), GY(10), GX(16), GY(22) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_network[] = {
	{ GO_TYPE_VERTEX4,	{ GX(10), GY(10), GX(4), GY(16), GX(6), GY(16), GX(11), GY(11) } },
	{ GO_TYPE_VERTEX4,	{ GX(4), GY(16), GX(6), GY(16), GX(11), GY(21), GX(10), GY(22) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(11), GY(16), GX(1) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(1) } },
	{ GO_TYPE_FILL_CIRCLE,	{ GX(21), GY(16), GX(1) } },
	{ GO_TYPE_VERTEX4,	{ GX(22), GY(10), GX(28), GY(16), GX(26), GY(16), GX(21), GY(11) } },
	{ GO_TYPE_VERTEX4,	{ GX(26), GY(16), GX(28), GY(16), GX(22), GY(22), GX(21), GY(21) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_clock[] = {
	{ GO_TYPE_FILL_CIRCLE,	{ GX(16), GY(16), GX(11) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_REVERSE }, },
	{ GO_TYPE_FILL_BOX,	{ GX(15), GY(8), GX(2), GY(9) } },
	{ GO_TYPE_FILL_BOX,	{ GX(17), GY(15), GX(6), GY(2) } },
	{ GO_TYPE_MODE,		{ GRP_DRAWMODE_NORMAL }, },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_list[] = {
	{ GO_TYPE_FILL_BOX,	{ GX(5), GY(8), GX(22), GY(4) } },
	{ GO_TYPE_FILL_BOX,	{ GX(5), GY(14), GX(22), GY(4) } },
	{ GO_TYPE_FILL_BOX,	{ GX(5), GY(20), GX(22), GY(4) } },
	{ 0, { 0, 0, 0, 0 }}
};

const struct st_graph_object gobj_on[] = {
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 0 } },
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 1 } },
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 2 } },
	{ GO_TYPE_SECTOR,	{ GXW(24), GY(16), GY(11), GY(10), 3 } },

	{ GO_TYPE_SECTOR,	{ GXW(20), GY(14), GY(3), GY(2), 1 } },
	{ GO_TYPE_SECTOR,	{ GXW(20), GY(14), GY(3), GY(2), 0 } },
	{ GO_TYPE_FILL_BOX,	{ GX(17), GY(14), GX(1), GY(4) } },
	{ GO_TYPE_FILL_BOX,	{ GX(22), GY(14), GX(1), GY(4) } },
	{ GO_TYPE_SECTOR,	{ GXW(20), GY(18), GY(3), GY(2), 2 } },
	{ GO_TYPE_SECTOR,	{ GXW(20), GY(18), GY(3), GY(2), 3 } },

	{ GO_TYPE_FILL_BOX,	{ GX(25), GY(11), GX(1), GY(10) } },
	{ GO_TYPE_VERTEX4,	{ GX(26), GY(11), GX(29), GY(18), GX(29), GY(21), GX(26), GY(14) } },
	{ GO_TYPE_FILL_BOX,	{ GX(29), GY(11), GX(1), GY(10) } },
	{ 0, { 0, 0, 0, 0 }}
};

/*
 */

const struct st_graph_object gobj_button[] = {
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, GOBJ_WIDTH, GOBJ_HEIGHT,  GOBJ_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

static void draw_button_object(short x, short y, const struct st_graph_object *gobj)
{
	draw_graph_object(x, y, gobj_button);

	draw_graph_object(x, y, gobj);
}

const struct st_graph_object gobj_wbutton[] = {
	{ GO_TYPE_ROUND_BOX,	{ 0, 0, GOBJ_WWIDTH, GOBJ_HEIGHT,  GOBJ_HEIGHT/8 } },
	{ 0, { 0, 0, 0, 0 }}
};

static void draw_wbutton_object(short x, short y, const struct st_graph_object *gobj)
{
	draw_graph_object(x, y, gobj_wbutton);

	draw_graph_object(x, y, gobj);
}

static void draw_objs(void)
{
	set_forecolor(RGB(255,255,255));
	set_backcolor(RGB(0,0,0));

	draw_button_object(32, 32, gobj_radio);
	draw_button_object(32+112, 32, gobj_cd);
	draw_button_object(32+112*2, 32, gobj_cdlist);
	draw_button_object(32+112*3, 32, gobj_ff);
	draw_button_object(32+112*4, 32, gobj_fr);
	draw_button_object(32+112*5, 32, gobj_repeat);
	draw_button_object(32+112*5, 32+112, gobj_shuffle);
	draw_button_object(32+112*2, 32+112, gobj_up);
	draw_button_object(32+112*3, 32+112, gobj_down);
	draw_button_object(32+112*4, 32+112, gobj_network);
	draw_button_object(32+112*2, 32+112*2, gobj_clock);
	draw_button_object(32+112*3, 32+112*2, gobj_list);
	draw_wbutton_object(32+112*0, 32+112*2, gobj_on);

	draw_wbutton_object(32, 32+112, gobj_playpause);
	draw_wbutton_object(32, 32+112*3, gobj_shuffle24);
}

int gt_task(void *arg)
{
	struct st_sysevent event;

	task_sleep(500);

	draw_objs();

	while(1) {
		if(get_event(&event, 50)) {
			switch(event.what) {
			case EVT_TOUCHSTART:
				draw_objs();
				break;
			}
		}
	}

	return 0;
}

#define SIZEOFAPPTS	(1024*8)
static struct st_tcb tcb;
static unsigned int stack[SIZEOFAPPTS/sizeof(unsigned int)] ATTR_STACK;

void startup_gobject_test(void)
{
	task_exec(gt_task, "gobject_test", TASK_PRIORITY_APP_HIGH, &tcb,
		  stack, SIZEOFAPPTS, 0);
}

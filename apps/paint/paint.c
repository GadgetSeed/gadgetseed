/** @file
    @brief	ペイント

    @date	2017.12.03
    @auther	Takashi SHUDO
*/

#include "tprintf.h"
#include "sysevent.h"
#include "graphics.h"
#include "task/syscall.h"

#include "ts_calib.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

#define COLOR_WHITE	RGB(240,240,250)
#define COLOR_BLACK	RGB(30,30,30)
#define COLOR_RED	RGB(255,30,30)
#define COLOR_ORANGE	RGB(255,165,0)
#define COLOR_YELLOW	RGB(255,255,0)
#define COLOR_GREENYELLOW	RGB(173,255,47)
#define COLOR_GREEN	RGB(0,200,0)
#define COLOR_BLUE	RGB(0,0,255)
#define COLOR_PURPLE	RGB(128,0,128)
#define COLOR_PINK	RGB(255,192,203)
#define COLOR_LIGHTBLUE	RGB(173,216,230)
#define COLOR_BROWN	RGB(165,42,42)

#define BACK_COLOR	COLOR_WHITE

#if GSC_GRAPHICS_DISPLAY_WIDTH >= 800
#define PALETTE_SIZE	32
#else
#define PALETTE_SIZE	16
#endif

static const unsigned int palette_color[] = {
	COLOR_WHITE,
	COLOR_BLACK,
	COLOR_RED,
	COLOR_ORANGE,
	COLOR_YELLOW,
	COLOR_GREENYELLOW,
	COLOR_GREEN,
	COLOR_BLUE,
	COLOR_PURPLE,
	COLOR_PINK,
	COLOR_LIGHTBLUE,
	COLOR_BROWN,
};

#define COLOR_NUM	(sizeof(palette_color)/sizeof(palette_color[0]))
#define PEN_NUM	3

static struct st_rect drawing_rect = { PALETTE_SIZE, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };
static struct st_rect colorsel_rect = { 0, 0, PALETTE_SIZE, COLOR_NUM * PALETTE_SIZE };
static struct st_rect pensel_rect = { 0, COLOR_NUM * PALETTE_SIZE, PALETTE_SIZE, (COLOR_NUM + PEN_NUM) * PALETTE_SIZE };
static struct st_rect unused_rect = { 0, (COLOR_NUM + PEN_NUM) * PALETTE_SIZE, PALETTE_SIZE, GSC_GRAPHICS_DISPLAY_HEIGHT };

static int pen_color_num = 1;
static int pen_pat_num = 0;

static int flg_draw = 0;
static short last_px, last_py;

static void draw_palette(int palette_num, int flg_sel)
{
	struct st_box cbox;

	cbox.sur.width = PALETTE_SIZE;
	cbox.sur.height = PALETTE_SIZE;
	cbox.pos.x = 0;
	cbox.pos.y = palette_num * PALETTE_SIZE;
	set_forecolor(palette_color[palette_num]);
	draw_fill_box(&cbox);
	set_forecolor(COLOR_BLACK);
	draw_box(&cbox);

	if(flg_sel != 0) {
		cbox.pos.x += 1;
		cbox.pos.y += 1;
		cbox.sur.width -= 2;
		cbox.sur.height -= 2;
		set_forecolor(COLOR_BLACK);
		draw_box(&cbox);
	}
}

static void draw_palette_select(void)
{
	int i;

	for(i=0; i<COLOR_NUM; i++) {
		int flg_sel = 0;
		if(i == pen_color_num) {
			flg_sel = 1;
		}
		draw_palette(i, flg_sel);
	}
}

static void draw_pen_point(short x, short y, int pat)
{
	draw_fill_circle(x, y, pat * pat);
}

static void draw_pen_line(short x0, short y0, short x1, short y1, int pat)
{
	int i, j;

	switch(pat) {
	case 0:
		draw_line(x0, y0, x1, y1);
		break;

	case 1:
		for(j=0; j<2; j++) {
			for(i=0; i<2; i++) {
				draw_line(x0+i, y0+j, x1+i, y1+j);
			}
		}
		break;

	case 2:
#if 0
		for(j=-3; j<4; j++) {
			for(i=-3; i<4; i++) {
				draw_line(x0+i, y0+j, x1+i, y1+j);
			}
		}
#else
		draw_pen_point(x1, y1, pat);
#endif
		break;

	default:
		break;
	}
}

static void draw_pen(int num, int flg_sel)
{
	struct st_box cbox;

	cbox.sur.width = PALETTE_SIZE;
	cbox.sur.height = PALETTE_SIZE;
	cbox.pos.x = 0;

	cbox.pos.y = (COLOR_NUM + num) * PALETTE_SIZE;
	set_forecolor(COLOR_WHITE);
	draw_fill_box(&cbox);
	set_forecolor(COLOR_BLACK);
	draw_box(&cbox);
	draw_pen_point(PALETTE_SIZE/2,
		       (COLOR_NUM + num) * PALETTE_SIZE + PALETTE_SIZE/2,
		       num);
	if(flg_sel != 0) {
		cbox.pos.x += 1;
		cbox.pos.y += 1;
		cbox.sur.width -= 2;
		cbox.sur.height -= 2;
		set_forecolor(COLOR_BLACK);
		draw_box(&cbox);
	}
}

static void draw_pen_select(void)
{
	int i;

	for(i=0; i<PEN_NUM; i++) {
		int flg_sel = 0;

		if(i == pen_pat_num) {
			flg_sel = 1;
		}
		draw_pen(i, flg_sel);
	}
}

static void draw_paint_screen(void)
{
	set_forecolor(BACK_COLOR);
	draw_fill_rect(&drawing_rect);
	draw_fill_rect(&pensel_rect);
	draw_fill_rect(&unused_rect);

	draw_palette_select();
	draw_pen_select();
}

int paint_proc(struct st_sysevent *event)
{
	switch(event->what) {
	case EVT_TOUCHSTART:
		DTPRINTF(0x01, "S %3d,%3d\n", event->pos_x, event->pos_y);
		if(is_point_in_rect(event->pos_x, event->pos_y, &colorsel_rect) != 0) {
			DTPRINTF(0x01, "COLOR_SEL %3d,%3d\n", event->pos_x, event->pos_y);
			clear_clip_rect();
			draw_palette(pen_color_num, 0);
			pen_color_num = (event->pos_y / PALETTE_SIZE);
			draw_palette(pen_color_num, 1);
			set_forecolor(palette_color[pen_color_num]);
			set_clip_rect(&drawing_rect);
		} else
		if(is_point_in_rect(event->pos_x, event->pos_y, &pensel_rect) != 0) {
			DTPRINTF(0x01, "PEN_SEL %3d,%3d\n", event->pos_x, event->pos_y);
			clear_clip_rect();
			draw_pen(pen_pat_num, 0);
			pen_pat_num = ((event->pos_y - PALETTE_SIZE * COLOR_NUM) / PALETTE_SIZE);
			draw_pen(pen_pat_num, 1);
			set_forecolor(palette_color[pen_color_num]);
			set_clip_rect(&drawing_rect);
		} else {
			flg_draw = 1;
			last_px = event->pos_x;
			last_py = event->pos_y;
			draw_pen_point(event->pos_x, event->pos_y, pen_pat_num);
		}
		break;

	case EVT_TOUCHMOVE:
		DTPRINTF(0x01, "M %3d,%3d\n", event->pos_x, event->pos_y);
		if(flg_draw != 0) {
			draw_pen_line(last_px, last_py, event->pos_x, event->pos_y, pen_pat_num);
			last_px = event->pos_x;
			last_py = event->pos_y;
		}
		break;

	case EVT_TOUCHEND:
		DTPRINTF(0x01, "E %3d,%3d\n", event->pos_x, event->pos_y);
		draw_pen_point(event->pos_x, event->pos_y, pen_pat_num);
		flg_draw = 0;
		break;

	default:
		break;
	}

	return 0;
}

static int paint_task(char *arg)
{
	task_sleep(100);

	draw_paint_screen();

	set_clip_rect(&drawing_rect);
	set_forecolor(palette_color[pen_color_num]);

	while(1) {
		struct st_sysevent event;

		if(get_event(&event, 50)) {
			paint_proc(&event);
		}
	}

	return 0;
}

static struct st_tcb tcb;
#define SIZEOFSTACK	(1024*4)
static unsigned int stack[SIZEOFSTACK/sizeof(unsigned int)];

void startup_paint(void)
{
#if defined(GSC_DEV_ENABLE_LCD_ILI9341) || defined(GSC_DEV_ENABLE_LCD_HX8357D)
	startup_ts_calib();
#endif

	task_exec(paint_task, "paint", TASK_PRIORITY_APP_HIGH, &tcb,
		  stack, SIZEOFSTACK, 0);
}

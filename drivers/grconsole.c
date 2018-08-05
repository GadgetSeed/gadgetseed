/** @file
    @brief	グラフィックコンソールドライバ

    グラフィックコンソールドライバはグラフィックライブラリを使用したコ
    ンソールドライバです。<br>

    register()実行時にコンソールに使用するフォント名を指定することが出
    来ます。省略時はフォント名"8x16"を使用ます。<br>

    register()時に指定されたフォントにより行幅、行高さが決定されます。

    グラフィックコンソールドライバは実験的なドライバであり未完成でです。

    @date	2008.01.04
    @author	Takashi SHUDO

    @todo 漢字表示を可能にする。
*/

#include "device.h"
#include "graphics.h"
#include "font.h"
#include "tkprintf.h"

//#define DEBUGKLVL 1
#include "dkprintf.h"


#define SCREEN_WIDTH	GRAPH_WIDTH
#define SCREEN_HEIGHT	GRAPH_HEIGHT
#define CONSOLE_WIDTH	(SCREEN_WIDTH/fnt_width)
#define CONSOLE_HEIGHT	(SCREEN_HEIGHT/fnt_height)

static short screen_width;
static short screen_height;
static short console_width;
static short console_height;

static short fnt_width;
static short fnt_height;
static long grc_addr;
static short cur_x;
static short cur_y;
static unsigned char esc;
static struct st_fontset *fset;
static struct st_rect screct;
static struct st_rect blrect;
#ifdef GSC_FONTS_ENABLE_KANJI
#ifdef GSC_FONTS_ENABLE_FONT_JISKAN16
static const char def_font[] = "jiskan16";
#else
static const char def_font[] = "8x16";
#endif
#else
static const char def_font[] = "8x16";
#endif

extern const struct st_device grconsole_device;

static int grconsole_register(struct st_device *dev, char *param)
{
	grc_addr = 0;
	cur_x = 0;
	cur_y = 0;
	esc = 0;
	char *fname = (char *)def_font;

	if(param != 0) {
		fname = param;
	}

	fset = set_font_by_name(fname);

	if(fset == 0) {
		SYSERR_PRINT("Cannot set font \"%s\".\n", fname);
		return -1;
	}

	fnt_width = fset->font->width;
	fnt_height = fset->font->height;

	get_screen_info(&screen_width, &screen_height);
	console_width = screen_width/fnt_width;
	console_height = screen_height/fnt_height;

	screct.left = 0;
	screct.right = screen_width;
	screct.top = 0;
	screct.bottom = screen_height;

	blrect.left = 0;
	blrect.right = screen_width;
	blrect.top = fnt_height * (console_height - 1);
	blrect.bottom = fnt_height * console_height;

	tkprintf("Console \"%s\" %dx%d, font \"%s\"\n", grconsole_device.name,
		 console_width, console_height, fname);

	return 0;
}

static void draw_cursor(unsigned char on)
{
	struct st_rect rect;

	rect.left = cur_x * fnt_width;
	rect.right = (cur_x + 1) * fnt_width;
	rect.top = cur_y * fnt_height;
	rect.bottom = (cur_y + 1) * fnt_height;

	if(on) {
		set_draw_mode(GRP_DRAWMODE_NORMAL);
	} else {
		set_draw_mode(GRP_DRAWMODE_REVERSE);
	}

	draw_fill_rect(&rect);
}

static void next_line(void)
{
	cur_y ++;
	if(cur_y >= console_height) {
		cur_y = (console_height - 1);
		clear_clip_rect();
		scroll_rect_v(&screct, -fnt_height);
		set_draw_mode(GRP_DRAWMODE_REVERSE);
		draw_fill_rect(&blrect);
		set_draw_mode(GRP_DRAWMODE_NORMAL);
	}
}

static void draw_ch(unsigned short x, unsigned short y, unsigned short ch)
{
	draw_char(x * fnt_width, y * fnt_height, ch);
}

static void put_char(unsigned char ch)
{
	switch(ch) {
	case '\r':
		draw_cursor(0);
		cur_x = 0;
		return;
		break;

	case '\n':
		cur_x = 0;
		next_line();
		break;

	case 0x08: // CTRL-H
		//draw_cursor(0);
		cur_x --;
		if(cur_x < 0) {
			cur_x = (console_width - 1);
			cur_y --;
			if(cur_y < 0) {
				cur_y = 0;
			}
		}
		break;

	default:
		if(cur_x >= (console_width-1)) {
			cur_x = 0;
			next_line();
		}
		draw_ch(cur_x, cur_y, ch);
		cur_x ++;
		if(cur_x >= console_width) {
			cur_x = 0;
			next_line();
		}
	}

	draw_cursor(1);
}

/*
 *
 */

static int grconsole_open(struct st_device *dev)
{
	draw_cursor(1);

	return 0;
}

static int grconsole_ioctl(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	switch(com) {
	case 0x2000:
		grconsole_register(dev, (char *)param);
		break;
	}

	return 0;
}

static int grconsole_putc(struct st_device *dev, unsigned char data)
{
	int i;

	switch(esc) {
	case 0:
		if(data == 0x1b) { // ESC
			esc = 1;
		} else {
			put_char(data);
		}
		break;
	case 1:
		if(data == '[') {
			esc = 2;
		} else {
			esc = 0;
		}
		break;

	case 2:
		switch(data) {
		case 'K': // 右消す
			draw_cursor(1);
			for(i=cur_x; i<console_width; i++) {
				draw_ch(i, cur_y, ' ');
			}
			draw_cursor(1);
			break;
		}
		esc = 0;
		break;
	}

//	#include "tprintf.h"
//	tprintf("x=%d, y=%d\n", cur_x, cur_y);

	return 1;
}

static int grconsole_seek(struct st_device *dev, int offset, int whence)
{
	switch(whence) {
	case SEEK_SET:
		grc_addr = offset;
		break;

	case SEEK_CUR:
		grc_addr += offset;
		break;

	case SEEK_END:
	default:
		return -1;
	}

	cur_x = grc_addr % console_width;
	cur_y = grc_addr / console_height;

	return grc_addr;
}

const struct st_device grconsole_device = {
	.name		= "grcon",
	.explan		= "Graphic console",
	.register_dev	= grconsole_register,
	.open		= grconsole_open,
	.ioctl		= grconsole_ioctl,
	.putc		= grconsole_putc,
	.seek		= grconsole_seek,
};

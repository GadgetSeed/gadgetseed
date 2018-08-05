/** @file
    @brief	ファイル操作実行

    @date	2017.11.20
    @author	Takashi SHUDO
*/

#include "shell.h"
#include "tprintf.h"
#include "file.h"
#include "graphics.h"
#include "sysevent.h"
#include "charcode.h"

static uchar cmd[FF_MAX_LFN + 1];
static unsigned char fname[FF_MAX_LFN + 1];
static unsigned char cname[FF_MAX_LFN + 1];	// UTF-8ファイル名

static int do_mp3_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	sjisstr_to_utf8str(cname, str, FF_MAX_LFN);
	tprintf("Play MP3 File \"%s\"\n", cname);
	tsprintf((char *)cmd, "sound %s mp3 %s", arg, fname);
	rt = exec_command(cmd);

	return rt;
}

static const struct st_file_operation mp3_operation = {
	"MP3", do_mp3_file
};


static int do_m4a_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	sjisstr_to_utf8str(cname, (unsigned char *)str, FF_MAX_LFN);
	tprintf("Play M4A File \"%s\"\n", cname);
	tsprintf((char *)cmd, "sound %s m4a %s", arg, fname);
	rt = exec_command(cmd);

	return rt;
}

static const struct st_file_operation m4a_operation = {
	"M4A", do_m4a_file
};


extern void draw_filemanager(void);

static void wait_next_event(void)
{
	while(1) {
		struct st_sysevent event;
		if(get_event(&event, 50)) {
			switch(event.what) {
			case EVT_TOUCHSTART:
			case EVT_KEYDOWN:
				goto end;
				break;

			default:
				break;
			}
		}
	}

end:
	return;
}

static int do_jpeg_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	sjisstr_to_utf8str(cname, (unsigned char *)str, FF_MAX_LFN);
	tprintf("Display JPEG File \"%s\"\n", cname);
	tsprintf((char *)cmd, "graph jpeg %s", fname);

	{
		struct st_rect frect = { 0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };
		set_forecolor(RGB(0, 0, 0));
		draw_fill_rect(&frect);
	}

	rt = exec_command(cmd);

	wait_next_event();

	draw_filemanager();

	return rt;
}

static const struct st_file_operation jpg_operation = {
	"JPG", do_jpeg_file
};


static int do_png_file(uchar *str, uchar *arg)
{
	int rt = 0;

	escaped_str(fname, str);
	sjisstr_to_utf8str(cname, (unsigned char *)str, FF_MAX_LFN);
	tprintf("Display PNG File \"%s\"\n", cname);
	tsprintf((char *)cmd, "graph png %s", fname);

	{
		struct st_rect frect = { 0, 0, GSC_GRAPHICS_DISPLAY_WIDTH, GSC_GRAPHICS_DISPLAY_HEIGHT };
		set_forecolor(RGB(0, 0, 0));
		draw_fill_rect(&frect);
	}

	rt = exec_command(cmd);

	wait_next_event();

	draw_filemanager();

	return rt;
}

static const struct st_file_operation png_operation = {
	"PNG", do_png_file
};


#include "batch.h"

static int do_bat_file(uchar *str, uchar *arg)
{
	int rt = exec_batch(str);

	return rt;
}

static const struct st_file_operation bat_operation = {
	"BAT", do_bat_file
};


const struct st_file_operation * const file_operation[] = {
	&mp3_operation,
	&m4a_operation,
	&jpg_operation,
	&png_operation,
	&bat_operation,
	0
};

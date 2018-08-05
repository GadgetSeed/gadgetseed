/** @file
    @brief	ファイルリスト表示

    @date	2017.11.18
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "str.h"
#include "charcode.h"
#include "tprintf.h"
#include "graphics.h"
#include "graphics_object.h"
#include "font.h"
#include "memory.h"
#include "key.h"

#include "ui_button.h"
#include "ui_selectlist.h"

#include "filemanager.h"
#include "filelist_view.h"
#include "filelist_data.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


#define LIST_ITEM_GAP	8

#define MAX_FNAME_LEN	255

// Config
// #define LONG_TIME_STRING
// #define LONG_FSIZE_STRING

#ifdef LONG_DATETIME_STRING
#define DATETIME_STR		"0000/00/00 00:00:00"
#else
#define DATETIME_STR		"00/00/00"
#endif
#define DATETIME_STR_LEN	(sizeof(DATETIME_STR)-1)

#ifdef LONG_FSIZE_STRING
#define FSIZE_STR	"000.00"
#define FSIZE_DIR_STR	"      "
#else
#define FSIZE_STR	"0000B"
#define FSIZE_DIR_STR	"     "
#endif
#define FSIZE_STR_LEN	(sizeof(FSIZE_STR)-1)

#define FNAME_STR_LEN	48
#define DIR_STR	"-DIR-"
#define DIR_STR_LEN	(sizeof(DIR_STR)-1)

static short fsize_str_width;
static short fdate_str_width;

static uchar *fsize2str(long size)
{
	static uchar szstr[FSIZE_STR_LEN + 1];

#ifdef LONG_FSIZE_STRING
	if(size >= 1024L*1024) {
		int scale = (1024L*1024);
		tsprintf((char *)szstr, "%3ld.%1ldM", size/scale, (size-((size/scale)*scale))/100);
	} else
	if(size >= 1024) {
		int scale = 1024L;
		tsprintf((char *)szstr, "%3ld.%1ldK", size/scale, (size-((size/scale)*scale))/100);
	} else {
		tsprintf((char *)szstr, "%5ldB", size);
	}

	return szstr;
#else
	if(size >= 1024L*1024) {
		tsprintf((char *)szstr, "%4ldM", size/(1024L*1024));
	} else
	if(size >= 1024) {
		tsprintf((char *)szstr, "%4ldK", size/1024);
	} else {
		tsprintf((char *)szstr, "%4ldB", size);
	}

	return szstr;
#endif
}

static uchar *fdate2str(unsigned short date, unsigned short time)
{
	static uchar dtstr[DATETIME_STR_LEN + 1];

#ifdef LONG_DATETIME_STRING
	tsprintf((char *)dtstr, "%04d/%02d/%02d %02d:%02d:%02d",
		 (int)(date >> 9) + 1980,
		 (int)(date >> 5) & 15,
		 (int)(date & 31),
		 (int)(time >> 11),
		 (int)(time >> 5) & 63,
		 (int)time & 63);

	return dtstr;
#else
	static DWORD now_time;

	if(date == (now_time >> 16)) {
		tsprintf((char *)dtstr, "%02d:%02d:%02d",
			 (int)(time >> 11),
			 (int)(time >> 5) & 63,
			 (int)time & 63);
	} else {
		tsprintf((char *)dtstr, "%02d/%02d/%02d",
			 (int)(date >> 9) + 1980 - 2000,
			 (int)(date >> 5) & 15,
			 (int)date & 31);
	}

	return dtstr;
#endif
}

#ifdef GSC_FONTS_ENABLE_FONT_JISKAN24
#define ICON_WIDTH		32
#else
#define ICON_WIDTH		24
#endif

extern const struct st_bitmap file_icon;
extern const struct st_bitmap folder_icon;

#define FNAME_WIDTH (GSC_GRAPHICS_DISPLAY_WIDTH - SCRBAR_WIDTH - ICON_WIDTH)

void draw_file_item(struct st_ui_selectlist *selectlist, struct st_box *box, int item_num, int flg_select)
{
	struct st_ui_selectlist *sl = selectlist;
	struct st_filelist_data *fd = sl->userdata;
	struct st_fileinfo **fi = fd->fileinfo_index;
#if 0
	uchar str[MAX_FNAME_LEN + 1];
#endif
	FILINFO *file_info = &(fi[item_num]->file_info);
	short fheight = font_height();

	DTPRINTF(0x01, "%3d (%3d,%3d) %3d,%3d %d %s\n",
		 item_num, box->pos.x, box->pos.y, box->sur.width, box->sur.height, flg_select,
		 sj2utf8((uchar *)fi[item_num]->file_info.fname));

	if(flg_select == 0) {
		draw_graph_object(box->pos.x, box->pos.y, sl->normal_view);
	} else {
		draw_graph_object(box->pos.x, box->pos.y, sl->select_view);
	}

	if(file_info->fattrib & AM_DIR) {
		draw_bitmap(box->pos.x, box->pos.y + (fheight-folder_icon.height)/2, (struct st_bitmap *)&folder_icon);
#if 0
		tsprintf((char *)str, fd->file_print_form,
			 (uchar *)file_info->fname,
			 fdate2str(file_info->fdate, file_info->ftime),
			 //"  DIR"
			 "     "
			 );
#endif
	} else {
		draw_bitmap(box->pos.x, box->pos.y + (fheight-folder_icon.height)/2, (struct st_bitmap *)&file_icon);
#if 0
		tsprintf((char *)str, fd->file_print_form,
			 (uchar *)file_info->fname,
			 fdate2str(file_info->fdate, file_info->ftime),
			 fsize2str(file_info->fsize));
#endif
	}

#if 0
	draw_str(box->pos.x + ICON_WIDTH, box->pos.y, sj2utf8(str));
#else
	draw_fixed_width_str(box->pos.x + ICON_WIDTH, box->pos.y,
			     sj2utf8((uchar *)file_info->fname),
			     FNAME_WIDTH-fdate_str_width-fsize_str_width);

	draw_fixed_width_str(box->pos.x + ICON_WIDTH + FNAME_WIDTH-fdate_str_width-fsize_str_width,
			     box->pos.y,
			     fdate2str(file_info->fdate, file_info->ftime),
			     fdate_str_width);

	draw_fixed_width_str(box->pos.x + ICON_WIDTH + FNAME_WIDTH-fsize_str_width,
			     box->pos.y,
			     fsize2str(file_info->fsize),
			     fsize_str_width);
#endif
}

void draw_filelist_view(struct st_filelist_view *filelist_view)
{
	struct st_filelist_view *fv = filelist_view;
	uchar str[MAX_FNAME_LEN + 1];

	draw_graph_object(fv->info_area.pos.x, fv->info_area.pos.y, fv->info_view);
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(fv->info_area));

	set_font_by_name(fv->font_name);
	set_draw_mode(GRP_DRAWMODE_NORMAL);
	tsprintf((char *)str, fv->filelist_data.dir_print_form, (uchar *)fv->path);
	draw_str(fv->info_area.pos.x, fv->info_area.pos.y, sj2utf8(str));

	draw_graph_object(fv->list_area.pos.x, fv->list_area.pos.y, fv->list_view);
	set_draw_mode(GRP_DRAWMODE_REVERSE);
	draw_fill_box(&(fv->list_area));

	set_draw_mode(GRP_DRAWMODE_NORMAL);
	draw_ui_selectlist(&(fv->selectlist));
}

static void make_path(uchar *dst, uchar *src1, uchar *src2)
{
	int i;

	strncopy(dst, src1, FF_MAX_LFN);
	for(i=0; i<MAX_PATH_LEN; i++) {
		if(dst[i] == 0) {
			dst[i] = '/';
			strncopy(&dst[i+1], src2, FF_MAX_LFN);
			break;
		}
	}
}

static int up_path(uchar *path)
{
	int i;

	for(i=strleng(path); i>=0; i--) {
		if(path[i] == '/') {
			path[i] = 0;
			return 1;
			break;
		}
	}

	return 0;
}

static int file_filter(FILINFO *finfo)
{
	/*
	  先頭一文字が'.'は表示しない
	 */

	if(finfo->fname[0] == '.') {
		return 1;
	} else {
		return 0;
	}
}

static int prepare_filelist(struct st_filelist_view *filelist_view, uchar *path)
{
	struct st_filelist_view *fv = filelist_view;
	int file_count;
	int i;
	struct st_fileinfo *fl;
	struct st_fileinfo **fi;

	strncopy(fv->path, path, MAX_PATH_LEN);
	file_count = create_filelist_data(&(fv->fileinfo_list), fv->path, file_filter);

	if(file_count != 0) {
		fl = fv->fileinfo_list.next;
		fi = alloc_memory(sizeof(struct st_fileinfo *)*file_count);
		if(fi != 0) {
			fv->filelist_data.fileinfo_index = fi;

			for(i=0; i<file_count; i++) {
				fi[i] = fl;
				fl = fl->next;
			}

			for(i=0; i<file_count; i++) {
				DTPRINTF(0x01, "%3d %s\n", i, sj2utf8((uchar *)fi[i]->file_info.fname));
			}
		} else {
			file_count = -1;
		}
	}

	fv->selectlist.userdata = &(fv->filelist_data);

	set_param_ui_selectlist(&(fv->selectlist), file_count, 0, 0);

	return file_count;
}

static void dispose_filelist(struct st_filelist_view *filelist_view)
{
	struct st_filelist_view *fv = filelist_view;

	free_memory(fv->filelist_data.fileinfo_index);

	dispose_filelist_data(&(fv->fileinfo_list));
}


static int movedir_filelist_view(struct st_filelist_view *filelist_view, uchar *path)
{
	struct st_filelist_view *fv = filelist_view;
	int rtn = FLV_EVT_NONE;

	dispose_filelist(fv);
	if(prepare_filelist(fv, path)) {
		draw_filelist_view(fv);
		rtn = FLV_EVT_MOVE_DIR;
	}

	return rtn;
}


void updir_filelist_view(struct st_filelist_view *filelist_view)
{
	struct st_filelist_view *fv = filelist_view;

	if(up_path(fv->path) != 0) {
		if(fv->filelist_data.dir_depth != 0) {
			fv->filelist_data.dir_depth --;
			DTPRINTF(0x01, "DIR_DEPTH %d, NUM %d\n",
				fv->filelist_data.dir_depth,
				fv->selectlist.select_item_num);
			movedir_filelist_view(fv, fv->path);
			set_select_item_num_ui_selectlist(&(fv->selectlist),
						       fv->filelist_data.select_num[fv->filelist_data.dir_depth]);
		}
	}
}

void downdir_filelist_view(struct st_filelist_view *filelist_view)
{
	struct st_filelist_view *fv = filelist_view;
	struct st_fileinfo **fi = fv->filelist_data.fileinfo_index;
	FILINFO *finfo = &(fi[fv->selectlist.select_item_num]->file_info);
	DTPRINTF(0x01, "%s\n", sj2utf8((uchar *)finfo->fname));

	if(finfo->fattrib & AM_DIR) {
		// ディレクトリ移動
		uchar str[MAX_PATH_LEN+1];
		int select_num;

		DTPRINTF(0x01, "FNAME:%s\n", sj2utf8((uchar *)finfo->fname));
		make_path(str, fv->path, (uchar *)finfo->fname);
		DTPRINTF(0x01, "DIR:%s\n", sj2utf8((uchar *)str));
		select_num = fv->selectlist.select_item_num;

		if(fv->filelist_data.dir_depth < MAX_DIR_DEPTH) {
			fv->filelist_data.select_num[fv->filelist_data.dir_depth]
					= select_num;
			DTPRINTF(0x01, "DIR_DEPTH %d, NUM %d\n",
				 fv->filelist_data.dir_depth, select_num);
			fv->filelist_data.dir_depth ++;
			movedir_filelist_view(fv, str);
		}
	}
}

static int do_select_item(struct st_filelist_view *filelist_view)
{
	struct st_filelist_view *fv = filelist_view;
	struct st_fileinfo **fi = fv->filelist_data.fileinfo_index;

	int select_num = fv->selectlist.select_item_num;

	FILINFO *finfo = &(fi[select_num]->file_info);
	int rtn = FLV_EVT_NONE;

	DTPRINTF(0x01, "%s\n", sj2utf8((uchar *)finfo->fname));

	if(finfo->fattrib & AM_DIR) {
		uchar str[MAX_PATH_LEN+1];

		DTPRINTF(0x01, "FNAME:%s\n", sj2utf8((uchar *)finfo->fname));
		make_path(str, fv->path, (uchar *)finfo->fname);
		DTPRINTF(0x01, "DIR:%s\n", sj2utf8((uchar *)str));

		if(fv->filelist_data.dir_depth < MAX_DIR_DEPTH) {
			fv->filelist_data.select_num[fv->filelist_data.dir_depth]
					= select_num;
			DTPRINTF(0x01, "DIR_DEPTH %d, NUM %d\n",
				 fv->filelist_data.dir_depth, select_num);
			fv->filelist_data.dir_depth ++;
			rtn = movedir_filelist_view(fv, str);
		}
	} else {
		rtn = FLV_EVT_FILE_SECCOND_SEL;
	}

	return rtn;
}


static int last_sel_item = 0;

int filelist_view_proc(struct st_filelist_view *filelist_view, struct st_sysevent *event)
{
	struct st_filelist_view *fv = filelist_view;
	struct st_ui_selectlist_event sl_evt;
	int rtn = FLV_EVT_NONE;

	if(proc_ui_selectlist(&sl_evt, &(fv->selectlist), event) != 0) {
		if(last_sel_item == fv->selectlist.select_item_num) {
			rtn = do_select_item(fv);
		} else {
			// ファイル、ディレクトリ選択
			rtn = FLV_EVT_FILE_FIRST_SEL;
		}

		last_sel_item = fv->selectlist.select_item_num;
	}

	switch(event->what) {
	case EVT_KEYDOWN:
		switch(event->arg) {
		case KEY_GB_ENTER:
			rtn = do_select_item(fv);
			break;

		case KEY_GB_LEFT:
			updir_filelist_view(fv);
			break;

		case KEY_GB_RIGHT:
			downdir_filelist_view(fv);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return rtn;
}

void init_filelist_view(struct st_filelist_view *filelist_view)
{
	struct st_filelist_view *fv = filelist_view;
	unsigned short fw;
	int stlen;

	set_font_by_name(DEFAULT_FONT);
	fsize_str_width = str_width((uchar *)FSIZE_STR);
	fdate_str_width = str_width((uchar *)DATETIME_STR);

	fv->scrollbar.selectlist = &(fv->selectlist);
	set_font_by_name(fv->font_name);
	fw = font_width(' ');
	stlen = (fv->list_area.sur.width - ICON_WIDTH)/fw;

	tsprintf(fv->filelist_data.dir_print_form, "%%%ds", stlen);

	tsprintf(fv->filelist_data.file_print_form, "%%%ds %%s%%s",
		 (int)(stlen - 1 - DATETIME_STR_LEN - FSIZE_STR_LEN));

	fv->selectlist.item_height = font_height() + LIST_ITEM_GAP;
	fv->selectlist.scrollbar = &(fv->scrollbar);

	fv->filelist_data.dir_depth = 0;

	prepare_filelist(fv, (uchar *)"0:");
}

char *get_filelist_select_fname(struct st_filelist_view *filelist_view, char *fname, int len)
{
	struct st_filelist_view *fv = filelist_view;
	struct st_fileinfo **fi = fv->filelist_data.fileinfo_index;
	FILINFO *finfo = &(fi[fv->selectlist.select_item_num]->file_info);

	DTPRINTF(0x01, "FNAME:%s\n", sj2utf8((uchar *)finfo->fname));
//	tsnprintf(fname, len, "%s/%s", filelist_view->path, finfo->fname); // [TODO]
	tsprintf(fname, "%s/%s", filelist_view->path, finfo->fname);

	return fname;
}

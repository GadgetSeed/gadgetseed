/** @file
    @brief	仮想 color LCD ウィンドウ

    @date	2016.12.28
    @author	Takashi SHUDO
*/

#include <stdio.h>
#include <gtk/gtk.h>
#include <sys/types.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <gdk/gdkkeysyms.h>

#include "sysconfig.h"
#include "tkprintf.h"
#include "sysevent.h"
#include "key.h"
#include "graphics.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


#ifdef GSC_GRAPHICS_DOTSIZE	// $gsc エミュレータ用グラフィックデバイスの1ドットサイズ
#define DOTSIZE	GSC_GRAPHICS_DOTSIZE
#else
#define DOTSIZE 1
#endif

#define XSIZE	GSC_GRAPHICS_DISPLAY_WIDTH
#define YSIZE	GSC_GRAPHICS_DISPLAY_HEIGHT

#define DISP_WIDTH	(DOTSIZE*XSIZE)
#define DISP_HEIGHT	(DOTSIZE*YSIZE)

static GtkWidget *window;
static GtkWidget *canvas;

static GdkPixbuf *pixbuf[2];
static int disp_buf_num = 0;
static int draw_buf_num = 0;
static guchar *gb_ptr;
static int buf_width;
static int channels;
static int lcdv_w, lcdv_h;
static int vlcd_ready;
static int vlcd_dirty;
static pthread_mutex_t vlcd_mutex;

unsigned int vlcd_fore_color = RGB(255, 255, 255);
unsigned int vlcd_back_color = RGB(0, 0, 0);
static struct st_rect clip;
static short pen_x = 0;
static short pen_y = 0;

void vlcd_set_disp_frame(int num)
{
	switch(num) {
	case 0:
	case 1:
		disp_buf_num = num;
		break;
	}
}

void vlcd_set_draw_frame(int num)
{
	switch(num) {
	case 0:
	case 1:
		draw_buf_num = num;
		break;
	}
}

void vlcd_set_forecolor(unsigned long color)
{
	vlcd_fore_color = color;
}

void vlcd_set_backcolor(unsigned long color)
{
	vlcd_back_color = color;
}

void vlcd_set_rect(struct st_rect *rect)
{
	DKFPRINTF(0x01, "%d %d %d %d\n",
		  rect->left, rect->top, rect->right, rect->bottom);

	pthread_mutex_lock(&vlcd_mutex);

	clip = *rect;

	pen_x = rect->left;
	pen_y = rect->top;

	pthread_mutex_unlock(&vlcd_mutex);
}

void vlcd_reset_rect(void)
{
	pthread_mutex_lock(&vlcd_mutex);

	clip.left = 0;
	clip.right = XSIZE;
	clip.top = 0;
	clip.bottom = YSIZE;

	pen_x = clip.left;
	pen_y = clip.top;

	pthread_mutex_unlock(&vlcd_mutex);
}

static unsigned long read_point_lcd(int x, int y)
{
	DKFPRINTF(0x01, "(%d,%d) %08X\n", x, y);

	if(vlcd_ready == 0) {
		return 0;
	}

	gb_ptr	= gdk_pixbuf_get_pixels(pixbuf[draw_buf_num]);

	guchar *dp = (gb_ptr + buf_width*(y*DOTSIZE) +
		      (x*DOTSIZE)*channels);
	unsigned long rtn =
			(*(dp + 0) << 16) +
			(*(dp + 1) << 8) +
			(*(dp + 2) << 0);

	return rtn;
}

static void draw_point_lcd(int x, int y, unsigned long color)
{
	int i, j;

	DKFPRINTF(0x01, "(%d,%d) %08X\n", x, y, color);

	if(vlcd_ready == 0) {
		return;
	}

	gb_ptr	= gdk_pixbuf_get_pixels(pixbuf[draw_buf_num]);

	for(j=0; j<DOTSIZE; j++) {
		for(i=0; i<DOTSIZE; i++) {
			guchar *dp = (gb_ptr + buf_width*(y*DOTSIZE+j) +
				      (x*DOTSIZE+i)*channels);
			*(dp + 0) = RGB_TO_R(color);
			*(dp + 1) = RGB_TO_G(color);
			*(dp + 2) = RGB_TO_B(color);
		}
	}

	if(vlcd_dirty == 0) {
		vlcd_dirty = 1;
	}
}

void vlcd_draw_point(int x, int y)
{
	pthread_mutex_lock(&vlcd_mutex);

	draw_point_lcd(x, y, vlcd_fore_color);

	pthread_mutex_unlock(&vlcd_mutex);
}

unsigned int vlcd_read_point(void)
{
	unsigned int rtn = read_point_lcd(pen_x, pen_y);

	pen_x ++;
	if(pen_x > clip.right) {
		pen_x = clip.left;
		pen_y ++;
	}

	return rtn;
}

void vlcd_write_point(unsigned int color)
{
	pthread_mutex_lock(&vlcd_mutex);

	draw_point_lcd(pen_x, pen_y, color);

	pen_x ++;
	if(pen_x > clip.right) {
		pen_x = clip.left;
		pen_y ++;
	}

	pthread_mutex_unlock(&vlcd_mutex);
}

void vlcd_repeat_data(int len)
{
	int i;

	DKFPRINTF(0x01, "len = %d\n", len);

	for(i=0; i<len; i++) {
		vlcd_write_point(vlcd_fore_color);
	}
}

void vlcd_read_data(unsigned int *data, long len)
{
	int i;

	DKFPRINTF(0x01, "len = %d\r\n", (int)len);

	for(i=0; i<len; i++) {
		*data = vlcd_read_point();
		data ++;
	}
}

void vlcd_write_data(unsigned int *data, long len)
{
	int i;

	DKFPRINTF(0x01, "len = %d\r\n", (int)len);

	for(i=0; i<len; i++) {
		vlcd_write_point(*data);
		data ++;
	}

	pthread_mutex_lock(&vlcd_mutex);
	if(vlcd_dirty == 0) {
		vlcd_dirty = 1;
	}
	pthread_mutex_unlock(&vlcd_mutex);
}

void vlcd_fill(unsigned long data)
{
	int i, j;

	for(j=0; j<YSIZE; j++) {
		for(i=0; i<XSIZE; i++) {
			draw_point_lcd(i, j, data);
		}
	}

	pthread_mutex_lock(&vlcd_mutex);
	if(vlcd_dirty == 0) {
		vlcd_dirty = 1;
	}
	pthread_mutex_unlock(&vlcd_mutex);
}

static gboolean cb_expose_event(GtkWidget *widget,
				GdkEventExpose *event,
				gpointer user_data)
{
	GdkPixbuf *tpixbuf = (GdkPixbuf *)user_data;

//	gb_ptr	= gdk_pixbuf_get_pixels(tpixbuf);
	lcdv_w	= gdk_pixbuf_get_width(tpixbuf);
	lcdv_h	= gdk_pixbuf_get_height(tpixbuf);
	buf_width = gdk_pixbuf_get_rowstride(tpixbuf);
	channels = gdk_pixbuf_get_n_channels(tpixbuf);

//	printf("buf_width = %d\n", buf_width);
//	printf("channels = %d\n", channels);
//	printf("width=%d, height=%d\n", lcdv_w, lcdv_h);
#if 0
	gdk_draw_pixbuf(widget->window, NULL, tpixbuf,
			0, 0, 0, 0, -1, -1,
			GDK_RGB_DITHER_NONE, 0, 0);
#endif
	vlcd_ready = 1;

	pthread_mutex_lock(&vlcd_mutex);
	if(vlcd_dirty == 0) {
		vlcd_dirty = 1;
	}
	pthread_mutex_unlock(&vlcd_mutex);

	return TRUE;
}

static unsigned short gdkevent2gsevent(GdkEventKey *event)
{
	switch(event->keyval) {
	case GDK_Return:	return KEY_GB_ENTER;	break;
	case GDK_Escape:	return KEY_GB_ESC;	break;
	case GDK_BackSpace:	return KEY_GB_BS;	break;

	case GDK_Left:		return KEY_GB_LEFT;	break;
	case GDK_Right:		return KEY_GB_RIGHT;	break;
	case GDK_Up:		return KEY_GB_UP;	break;
	case GDK_Down:		return KEY_GB_DOWN;	break;

	case GDK_a:		return KEY_GB_A;	break;
	}

	return 0;
}

static void set_gs_event(struct st_sysevent *gsevent)
{
	push_event_interrupt(0, gsevent);
	lock_timer();
	set_event_interrupt(0);
	unlock_timer();
}

static gboolean cb_key_press(GtkWidget *widget,
			     GdkEventKey *event,
			     gpointer userdata)
{
	struct st_sysevent gsevent;

#ifdef LWDEBUG
	g_print("PRRESS  : keyval=%04x, status=%d, string=%s\r\n",
		event->keyval, event->state, event->string);
#endif

	gsevent.what = EVT_KEYDOWN;
	gsevent.arg = gdkevent2gsevent(event);
	set_gs_event(&gsevent);

	return FALSE;
}

static gboolean cb_key_release(GtkWidget *widget,
			       GdkEventKey *event,
			       gpointer userdata)
{
	struct st_sysevent gsevent;

#ifdef LWDEBUG
	g_print("RELEASE : keyval=%04x, status=%d, string=%s\r\n",
		event->keyval, event->state, event->string);
#endif

	gsevent.what = EVT_KEYUP;
	gsevent.arg = gdkevent2gsevent(event);
	set_gs_event(&gsevent);

	return FALSE;
}

static int flg_btn_press = 0;

static gboolean cb_button_press(GtkWidget *widget,
				GdkEventButton *event,
				gpointer userdata)
{
	struct st_sysevent gsevent;

#ifdef LWDEBUG
	g_print("B PRRESS  : x=%f, y=%f\r\n",
		event->x, event->y);
#endif

	flg_btn_press = 1;
	gsevent.what = EVT_TOUCHSTART;
	gsevent.pos_x = event->x;
	gsevent.pos_y = event->y;
	set_gs_event(&gsevent);

	return FALSE;
}

static gboolean cb_button_release(GtkWidget *widget,
				  GdkEventButton *event,
				  gpointer userdata)
{
	struct st_sysevent gsevent;

#ifdef LWDEBUG
	g_print("B RELEASE : x=%f, y=%f\r\n",
		event->x, event->y);
#endif

	flg_btn_press = 0;
	gsevent.what = EVT_TOUCHEND;
	gsevent.pos_x = event->x;
	gsevent.pos_y = event->y;
	set_gs_event(&gsevent);

	return FALSE;
}

static gboolean cb_motion_notify(GtkWidget *widget,
				 GdkEventButton *event,
				 gpointer userdata)
{
	struct st_sysevent gsevent;

	if(flg_btn_press != 0) {
		gsevent.what = EVT_TOUCHMOVE;
		gsevent.pos_x = event->x;
		gsevent.pos_y = event->y;
#ifdef LWDEBUG
		g_print("MOTION   : x=%f, y=%f\r\n",
			event->x, event->y);
#endif
		set_gs_event(&gsevent);
	}

	return FALSE;
}

static gint task_lcd_draw(gpointer data)
{
	block_timer_interrupt();

	if(vlcd_ready) {
		if(vlcd_dirty) {
			gdk_draw_pixbuf(canvas->window, NULL, pixbuf[disp_buf_num],
					0, 0, 0, 0, -1, -1,
					GDK_RGB_DITHER_NONE, 0, 0);

			pthread_mutex_lock(&vlcd_mutex);
			vlcd_dirty = 0;
			pthread_mutex_unlock(&vlcd_mutex);
		}
	}

	return TRUE;
}

static void *task_lcdwindow(void *arg)
{
	gtk_main();

	return 0;
}

int open_lcdwindow(int *argc, char ***argv)
{
	pthread_t thread_id;
	int status;

	vlcd_ready = 0;
	vlcd_dirty = 0;

	block_timer_interrupt();

	vlcd_reset_rect();

	gtk_init(argc, argv);

	pixbuf[0] = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
				DISP_WIDTH, DISP_HEIGHT);
	pixbuf[1] = gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8,
				DISP_WIDTH, DISP_HEIGHT);

	window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(window, DISP_WIDTH, DISP_HEIGHT);
	gtk_window_set_resizable(GTK_WINDOW (window), FALSE);
	g_signal_connect(G_OBJECT(window), "destroy",
			 G_CALLBACK(gtk_main_quit), NULL);

	canvas = gtk_drawing_area_new();
	gtk_widget_set_size_request(canvas,
				    gdk_pixbuf_get_width(pixbuf[0]),
				    gdk_pixbuf_get_height(pixbuf[0]));

	g_signal_connect(G_OBJECT(canvas), "expose-event",
			 G_CALLBACK(cb_expose_event), pixbuf[0]);
	g_signal_connect(G_OBJECT(canvas), "expose-event",
			 G_CALLBACK(cb_expose_event), pixbuf[1]);

	gtk_container_add(GTK_CONTAINER(window), canvas);

	gtk_widget_add_events(window, GDK_KEY_PRESS_MASK);
	g_signal_connect(G_OBJECT(window), "key-press-event",
			 G_CALLBACK(cb_key_press), NULL);
	g_signal_connect(G_OBJECT(window), "key-release-event",
			 G_CALLBACK(cb_key_release), NULL);

	g_signal_connect(G_OBJECT(window), "button-press-event",
			 G_CALLBACK(cb_button_press), NULL);
	g_signal_connect(G_OBJECT(window), "button-release-event",
			 G_CALLBACK(cb_button_release), NULL);
	g_signal_connect(G_OBJECT(window), "motion-notify-event",
			 G_CALLBACK(cb_motion_notify), NULL);
	gtk_widget_set_events(canvas, GDK_EXPOSURE_MASK
			      | GDK_LEAVE_NOTIFY_MASK
			      | GDK_BUTTON_PRESS_MASK
			      | GDK_BUTTON_RELEASE_MASK
			      | GDK_POINTER_MOTION_MASK
			      | GDK_POINTER_MOTION_HINT_MASK);

	gtk_widget_show_all(window);

	status = pthread_create(&thread_id, NULL, task_lcdwindow,
				(void *)NULL);
	if(status != 0) {
		fprintf(stderr, "pthread_create : %s",
			(char *)strerror(status));
	}

	gtk_timeout_add(20, (GtkFunction)task_lcd_draw, NULL);

	unblock_timer_interrupt();

	return 0;
}

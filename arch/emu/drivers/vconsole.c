/** @file
    @brief	仮想コンソールドライバ

    @date	2009.10.25
    @author	Takashi SHUDO
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <termios.h>

#include "fifo.h"
#include "device.h"
#include "task/event.h"
#include "task/syscall.h"

//#define DEBUG
//#define USE_CURSES

#ifdef USE_CURSES
#include <curses.h>
#include <signal.h>
#endif

static struct sigaction sigint;
static struct termios term, default_term;

static void exit_vconsole(int no)
{
	tcsetattr(fileno(stdin), TCSANOW, &default_term);

#ifdef USE_CURSES
	endwin();
#endif

	exit(0);
}

#define	MAXBUFSIZE	80

static struct st_fifo rfifo;
static unsigned char rbuf[MAXBUFSIZE];
static struct st_event rx_evq;

static void *task_vconsole(void *arg)
{
	int c;
	unsigned char rd;

	while(1) {
#ifdef USE_CURSES
		c = getch();
#else
		c = getchar();
#endif

		if(c != -1) {
#ifdef DEBUG
			printf("[%02X]", c);
#endif
			if(c == 0x0a) c = 0x0d;
			rd = (unsigned char)c;
			write_fifo(&rfifo, &rd, 1);
			lock_timer();
			event_wakeup_ISR(0, &rx_evq, 0);
			unlock_timer();
		}
	}

	return 0;
}

static int conemu_register(struct st_device *dev, char *param)
{
	pthread_t thread_id;
	int status;

	init_fifo(&rfifo, rbuf, MAXBUFSIZE);
	eventqueue_register(&rx_evq, "sci_rx", 0, 0, 0);
//	eventqueue_register_ISR(&rx_evq, "sci_rx", 0, 0, 0);

	tcgetattr(fileno(stdin), &term);
	default_term = term;

	term.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(fileno(stdin), TCSANOW, &term);

	block_timer_interrupt();
	status = pthread_create(&thread_id, NULL, task_vconsole, (void *)NULL);
	unblock_timer_interrupt();

	if(status != 0) {
		fprintf(stderr, "pthread_create : %s",
			(char *)strerror(status));
	}

	return 0;
}

static int conemu_getc(struct st_device *dev, unsigned char *rd)
{
	return read_fifo(&rfifo, rd, 1);
}

static int conemu_select(struct st_device *dev, unsigned int timeout)
{
	if(fifo_size(&rfifo) != 0) {
		return timeout;
	} else {
		return event_wait(&rx_evq, 0, timeout);
	}
}

static int conemu_putc(struct st_device *dev, unsigned char ch)
{
	putchar(ch);
#ifdef USE_CURSES
	refresh();
#endif

	return 1;
}

const struct st_device vconsole_device = {
	.name		= "tty",
	.explan		= "EMU Console",
	.register_dev	= conemu_register,
	.getc		= conemu_getc,
	.putc		= conemu_putc,
	.select		= conemu_select,
};

/*
  mon デバイス
*/

static int verror_register(struct st_device *dev, char *param)
{
	memset(&sigint, 0, sizeof(struct sigaction));
	sigint.sa_handler = exit_vconsole;
	sigint.sa_flags |= SA_NODEFER;

	if(sigaction(SIGINT, &sigint, NULL) != 0) {
		fprintf(stderr, "sigaction(2) error!\n");
	}

#ifdef USE_CURSES
	initscr();
	cbreak();
//	timeout(10);
	noecho();
	refresh();
#endif
	return 0;
}

static int verror_putc(struct st_device *dev, unsigned char ch)
{
	putchar(ch);
//	fprintf(stderr, "%c", ch);

	return 1;
}

const struct st_device verror_device = {
	.name		= "tty_low",
	.explan		= "EMU Error out",
	.register_dev	= verror_register,
	.putc		= verror_putc,
};

/*
 *
 */

static unsigned char f_monitor_getc(unsigned char *rd)
{
	int c;

#ifdef USE_CURSES
	c = getch();
#else
	c = getchar();
#endif

	if(c != -1) {
		*rd = (unsigned char)c;
		return 1;
	} else {
		return 0;
	}
}

unsigned char (* monitor_getc)(unsigned char *rd) = f_monitor_getc;

static void f_monitor_putci(struct st_device *dev, unsigned char ch)
{
	conemu_putc(dev, ch);
}

void (* monitor_putci)(struct st_device *dev, unsigned char ch) = f_monitor_putci;

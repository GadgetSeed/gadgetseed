/** @file
    @brief	newlib システムコール

    @date	2017.04.29
    @author	Takashi SHUDO
*/

#include "tkprintf.h"
#include "console.h"

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <reent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

//#define DEBUGKBITS 0x01
#include "dkprintf.h"


_ssize_t _read_r(struct _reent *r, int file, void *ptr, size_t len)
{
	return 0;
}

_ssize_t _write_r(struct _reent *r, int file, const void *ptr, size_t len)
{
	int i;
	const unsigned char *p;

	p = (const unsigned char *)ptr;

	for(i = 0; i < len; i++) {
		if(*p == '\n') {
			cputc('\r');
		}
		cputc(*p++);
	}

	return len;
}

void _fini(void)
{
	// [TODO]
}

int _close_r(struct _reent *r, int file)
{
	// [TODO]
	return 0;
}

_off_t _lseek_r(struct _reent *r, int file, _off_t ptr, int dir)
{
	// [TODO]
	return (_off_t)0;
}

int _fstat_r(struct _reent *r, int file, struct stat *st)
{
	// [TODO]
	st->st_mode = S_IFCHR;

	return 0;
}


extern char end[];		// .bssの最終アドレス、ヒープの先頭になる
extern char _heap_end[];	// ヒープの最終アドレス
static char *heap_ptr = NULL;	// 現在のヒープポインタ

void * _sbrk_r(struct _reent *_s_r, ptrdiff_t nbytes)
{
	char *base;

	// MUTEX Lock [TODO]
	DKPRINTF(0x01, "_sbrk_r reqest bytes %d\n", nbytes);

	if(heap_ptr == 0) {
		heap_ptr = end;
	}

	base = heap_ptr;

	if((heap_ptr + nbytes) > _heap_end) {
		errno = ENOMEM;
		SYSERR_PRINT("No Left Memory\n");
		SYSERR_PRINT("Request %8d, Left %p-%p(%d)\n", nbytes, base, _heap_end,
			     (unsigned int)_heap_end - (unsigned int)base);
		return (caddr_t) -1;
	}

	heap_ptr += nbytes;

	DKPRINTF(0x01, "_sbrk_r %8d %p-%p(%d)\n", nbytes, base, _heap_end,
		(unsigned int)_heap_end - (unsigned int)base);

	// MUTEX UnLock [TODO]

	return base;
}

unsigned int system_heap_size(void)
{
	return (unsigned int)_heap_end - (unsigned int)heap_ptr;
}

unsigned int system_heap_total_size(void)
{
	tkprintf("Heap area    : %p - %p (%d)\n", end, _heap_end,
		(unsigned int)_heap_end - (unsigned int)end);

	return (unsigned int)_heap_end - (unsigned int)end;
}

int isatty(int file)
{
	// [TODO]
	return 1;
}

int _getpid(int file)
{
	// [TODO]
	return 1;
}

int _isatty(int fd)
{
	return 1;
}

#if 0
void * _sbrk(ptrdiff_t incr)
{
	char *base;

	if(heap_ptr == 0) {
		heap_ptr = end;
	}

	base = heap_ptr;

	if((heap_ptr + incr) > _heap_end) {
		errno = ENOMEM;
		return (caddr_t) -1;
	}

	heap_ptr += incr;

	return base;
}

int _open(const char *path, int flags, ...)
{
	// [TODO]
	return 1;
}

int _close(int fd)
{
	// [TODO]
	return 0;
}

int _fstat(int fd, struct stat *st)
{
	st->st_mode = S_IFCHR;
	return 0;
}

int _lseek(int fd, off_t pos, int whence)
{
	return 0;
}

int _read(int fd, char *buf, size_t cnt)
{
	// [TODO]
	return 0;
}

int _write(int fd, const char *buf, size_t cnt)
{
	int i;

	for (i = 0; i < cnt; i++)
		cputc(buf[i]);

	return cnt;
}
#endif

/*
 *
 */

int _times(struct tms *buf)
{
	// [TODO]
	return -1;
}

int _init(struct tms *buf)
{
	// [TODO]
	return -1;
}

int _link(char *old, char *new)
{
	errno = EMLINK;
	return -1;
}

int _unlink(char *name)
{
	errno = ENOENT;
	return -1;
}

int _wait(int *status)
{
	errno = ECHILD;
	return -1;
}

int _execve(char *name, char **argv, char **env)
{
	errno = ENOMEM;
	return -1;
}

int _fork(void)
{
	errno = EAGAIN;
	return -1;
}

void _exit(int n)
{
	// [TODO]
	while(1);
}

int _kill(int pid, int sig)
{
	// [TODO]
	errno = EINVAL;
	return -1;
}

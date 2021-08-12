/** @file
    @brief	メモリ管理

    @date	2008.03.18
    @author	Takashi SHUDO

    @page memory_manage メモリ管理

    GadgetSeedはヒープメモリとヒープメモリを取得、開放するAPIがあります。

    GadgetSeedのヒープメモリ構成は以下の２種類よ選択することが出来ます。

    - newlibのAPIであるmalloc()、free()を使用する
    - GadgetSeedの独自APIを使用する

    newlibのAPIを使用する場合マクロ @ref GSC_MEMORY_HEAP_IS_NEWLIB を定義して下さい。
    この場合ヒープメモリエリアは .bss の最終アドレス以降となります。

    GadgetSeedの独自APIを使用する場合、ヒープメモリサイズはマクロ @ref GSC_MEMORY_HEAP_SIZE で定義して下さい。


    ---
    @section memory_api メモリ管理API

    include ファイル : memory.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | alloc_memory()		| @copybrief alloc_memory	|
    | free_memory()		| @copybrief free_memory	|
    | heap_total_size()		| @copybrief heap_total_size	|
    | heap_free_size()		| @copybrief heap_free_size	|

    GadgetSeedの独自APIを使用する場合、更に以下のAPIが使用できます。

    include ファイル : memory.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | memory_size()		| @copybrief memory_size	|
*/

#include "sysconfig.h"
#include "memory.h"
#include "tkprintf.h"
#include "str.h"

//#define DEBUGKBITS 0x10
#include "dkprintf.h"


#ifdef TEST
#include <stdio.h>
#define tprintf printf
#define eprintf printf
#endif

#ifndef GSC_MEMORY_HEAP_IS_NEWLIB	///< $gsc ヒープメモリ管理をnewlibで行う
#ifndef GSC_MEMORY_HEAP_SIZE
#define GSC_MEMORY_HEAP_SIZE 0x8000	///< $gsc ヒープメモリサイズ(newlibを使わない場合)
#endif

typedef union memhdr {
	struct {
		union memhdr *next;
		unsigned long size;
	} s;
	long align;
} memhdr;

#if 0
unsigned char gs_heap_mem[GSC_MEMORY_HEAP_SIZE] __attribute__ ((aligned(256)));
void *MEM_END = &gs_heap_mem[GSC_MEMORY_HEAP_SIZE];
#else
#define GHALIGN (sizeof(memhdr)*4)
unsigned char gs_heap_mem[(GSC_MEMORY_HEAP_SIZE/GHALIGN)*GHALIGN] __attribute__ ((aligned(GHALIGN)));
void *MEM_END = &gs_heap_mem[(GSC_MEMORY_HEAP_SIZE/GHALIGN)*GHALIGN];
#endif
void *MEM_START = gs_heap_mem;

memhdr *base;
memhdr *freep;
#else // GSC_MEMORY_HEAP_IS_NEWLIB
  #include <stdlib.h>
  #ifdef GSC_TARGET_SYSTEM_EMU
    unsigned int system_heap_size(void) { return 0; }
    unsigned int system_heap_total_size(void) { return 0; }
  #else
    extern unsigned int system_heap_size(void);
    extern unsigned int system_heap_total_size(void);
  #endif
#endif // GSC_MEMORY_HEAP_IS_NEWLIB

/**
   @brief	メモリ管理の初期化
*/
void init_memory(void)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	base = (memhdr *)(((unsigned long)MEM_START -
			   sizeof(memhdr)-1) & ~(sizeof(memhdr)-1));
	base->s.next = base + 1;
	base->s.size = 0;
	freep = base->s.next;
	freep->s.next = base;
	freep->s.size = ((char*)MEM_END - (char*)freep) / sizeof(memhdr);
#else
	tkprintf("GS Memory Alloc API is newlib API\n");
#endif
}

/**
   @brief	メモリ状態表示
*/
void display_memory_info(void)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	DKPRINTF(0x02, "### memhdr = %d\n", sizeof(memhdr));
	DKPRINTF(0x02, "### base = 0x%p\n", base);
	DKPRINTF(0x02, "### freep = 0x%p\n", freep);

	tkprintf("Heap area    : 0x%p - 0x%p\n",
		MEM_START, MEM_END);
	tkprintf("%ld K byte free\n", heap_total_size()/1024);
#else
	unsigned long ms = system_heap_total_size();
	tkprintf("%ld K byte free\n", ms/1024);
#endif
}

/**
   @brief	メモリを確保する

   @param[in] size	確保するメモリバイト数

   @return	確保したメモリのポインタ
*/
void * alloc_memory(unsigned int size)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	memhdr *p, *prevp = freep;
	unsigned int nunits;

	DKPRINTF(0x02, "### alloc size = %08lX(%ld)\n", size, size);
	DKPRINTF(0x02, "### freep = %p\n", freep);
	DKPRINTF(0x02, "### freep->s.next = %p\n", freep->s.next);
//	tkprintf("###? freep->s.next = %08lX\n", (unsigned long)freep->s.next);	//!!!
	nunits = (size + sizeof(memhdr)-1)/sizeof(memhdr) + 1;
	DKPRINTF(0x02, "### nunits = %08lX(%ld)\n", nunits, nunits);
	for(p = prevp->s.next; ; prevp = p, p = p->s.next) {
		DKPRINTF(0x02, "### p = %p\n", p);
		DKPRINTF(0x02, "### prevp = %p\n", prevp);
		DKPRINTF(0x02, "### p->s.size = %08lX(%ld)\n", p->s.size, p->s.size);
		if(p->s.size >= nunits) {
			break;
		}
		if(p == freep) {
			SYSERR_PRINT("Memory empty(size=%ld)\n", size);
			return 0;
		}
	}
	if(p->s.size == nunits) {
		prevp->s.next = p->s.next;
	} else {
		DKPRINTF(0x02, "### newmem(%ld)\n", nunits);
		DKPRINTF(0x02, "### p->s.size = %08lX(%ld)\n", p->s.size, p->s.size);
		p->s.size -= nunits;
		DKPRINTF(0x02, "### p->s.size = %08lX(%ld)\n", p->s.size, p->s.size);
		DKPRINTF(0x02, "### p = %p\n", p);
		p += p->s.size;
		DKPRINTF(0x02, "### p = %p\n", p);
		DKPRINTF(0x02, "### p->s.size = %08lX(%ld)\n", p->s.size, p->s.size);
		p->s.size = nunits;
		DKPRINTF(0x02, "### p->s.size = %08lX(%ld)\n", p->s.size, p->s.size);
		DKPRINTF(0x02, "### prevp = %p\n", prevp);
	}
	freep = prevp;
	DKPRINTF(0x02, "### freep = %p\n", freep);
//	memoryset((void *)(p+1), 0, size);	// calloc
	DKPRINTF(0x02, "### address = %p\n", (p + 1));

	DKPRINTF(0x01, "alloc ptr=%p size=%ld\n", (p + 1), size);

	return (void *)(p+1);
#else
	DKPRINTF(0x10, "alloc size = %08X(%u)\n", size, size);
	void *mp = malloc(size);
	DKPRINTF(0x10, "alloc addr = %p\n", mp);

	if(mp == 0) {
		SYSERR_PRINT("alloc error(size = %d)\n", size);
	}

	return mp;
#endif
}

/**
   @brief	確保したメモリを開放する

   @param[in] ptr	開放するメモリのポインタ
*/
void free_memory(void *ptr)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	memhdr *bp, *p;

	DKPRINTF(0x01, "free  ptr=%p\n", ptr);

	// ptr の範囲をチェック(すべき)
	DKPRINTF(0x02, "### free ptr = %p\n", ptr);
	DKPRINTF(0x02, "### freep = %p\n", freep);
	DKPRINTF(0x02, "### freep->s.next = %08lX\n", (unsigned long)freep->s.next);
//	tkrintf("###? freep->s.next = %08lX\n", (unsigned long)freep->s.next);	//!!!
	
	bp = (memhdr *)ptr -1;
	for(p = freep; !(bp > p && bp < p->s.next); p = p->s.next) {
		if(p >= p->s.next && (bp > p || bp < p->s.next)) {
			break;
		}
	}

	if(bp + bp->s.size == p->s.next) {
		bp->s.size += p->s.next->s.size;
		bp->s.next = p->s.next->s.next;
	} else {
		bp->s.next = p->s.next;
	}

	if(p + p->s.size == bp) {
		p->s.size += bp->s.size;
		p->s.next = bp->s.next;
	} else {
		p->s.next = bp;
	}
	freep = p;
#else
	DKPRINTF(0x10, "free addr = %p\n", ptr);
	free(ptr);
#endif
}

/**
   @brief	確保したメモリのバイト数を取得する

   @param[in] ptr	確保したメモリのポインタ

   @return	確保したメモリのバイト数
*/
unsigned long memory_size(void *ptr)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	return (((memhdr *)ptr -1)->s.size -1) * sizeof(memhdr);
#else
	return 0;	// [TODO]
#endif
}

/**
   @brief	未確保のメモリの合計バイト数を取得する

   @return	未確保のメモリの合計バイト数
*/
unsigned long heap_free_size(void)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	memhdr *p;
	unsigned long size = 0;

	for(p= freep->s.next; ; p = p->s.next) {
		size += p->s.size;
		if(p == freep) break;;
	}
	return size * sizeof(memhdr);
#else
	return system_heap_size();
#endif
}

/**
   @brief	全てのメモリのバイト数を取得する

   @return	全てのメモリのバイト数
*/
unsigned long heap_total_size(void)
{
#ifndef GSC_MEMORY_HEAP_IS_NEWLIB
	return (unsigned long)(MEM_END - MEM_START);
#else
	return system_heap_total_size();
#endif
}


//#ifndef TARGET_SYSTEM_EMU
#if 0
/*
 *
 */
#include <sys/reent.h>

void * _sbrk_r(struct _reent *r, int incr)
{
	//tprintf("incr = %d 0x%08X\n", incr, incr);
	return alloc_memory(incr);
}

void * _sbrk(ptrdiff_t incr)
{
	return alloc_memory(incr);
}

#endif // TARGET_SYSTEM_EMU


#ifdef TEST
/*
 * テスト
 */
#define MEMSIZE	0x10000
static unsigned char mem[MEMSIZE];

void * memalloc(unsigned long size)
{
	void *mp;
	printf("MALLOC(%ld)\n", size);
	mp = alloc_memory(size);
	if(mp == 0) {
		printf("Error\n");
	} else {
		printf("pointer : %08X\n", mp - MEM_START);
		printf("size = %ld\n", memory_size(mp));
		printf("free = %ld\n", heap_free_size());
	}
	return mp;
}

void memfree(void *ptr)
{
	printf("FREE(%08x)\n", ptr - MEM_START);
	free_memory(ptr);
	printf("free = %ld\n", heap_free_size());
}

void memlist(void)
{
	memhdr *p, *prevp = freep;
	for(p= prevp->s.next; ; prevp = p, p = p->s.next) {
		if(p == freep) return;
		tprintf("alloced : %08lx (%ld)\n", (unsigned long)p+1,
			(unsigned long)p->s.size*sizeof(memhdr));
	}
}

int main(int argc, char *argv[])
{
	void *mp[10];
	int i;

	for(i=0; i< MEMSIZE; i++) {
		mem[i] = 0xff;
	}
	
	MEM_START = mem;
	MEM_END = &mem[MEMSIZE];

	init_memory();

	printf("%ld byte free\n", heap_free_size());

	printf("base = %08lx\n", (unsigned long)base);
	printf("freep = %08lx\n", (unsigned long)freep);

	mp[0] = memalloc(100);
	mp[1] = memalloc(100);
	memfree(mp[0]);
	printf("free\n");
	mp[2] = memalloc(100);
	mp[3] = memalloc(1000);
	mp[4] = memalloc(10000);
	memfree(mp[2]);
	mp[5] = memalloc(10000);
	memlist();
//	memfree(mp[1]);
	memfree(mp[4]);
	memfree(mp[3]);
	memfree(mp[5]);

	mp[6] = memalloc(20000);
	mp[7] = memalloc(30000);
	memlist();
	mp[8] = memalloc(15280);
	memlist();
	mp[9] = memalloc(100);

	return 0;
}
#endif

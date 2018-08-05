/** @file
    @brief	メモリコマンド

    @date	2007.04.22
    @author	Takashi SHUDO

    @section mem_command memコマンド

    mem コマンドには以下のサブコマンドがあります。

    | サブコマンド	| 機能				| 詳細			|
    |:------------------|:------------------------------|:----------------------|
    | dump		| @copybrief com_mem_dump	| @ref com_mem_dump	|
    | fill		| @copybrief com_mem_fill	| @ref com_mem_fill	|
    | edit		| @copybrief com_mem_edit	| @ref com_mem_edit	|
    | check		| @copybrief com_mem_check	| @ref com_mem_check	|
*/

#include "asm.h"
#include "shell.h"
#include "lineedit.h"
#include "console.h"
#include "timer.h"
#include "str.h"
#include "tprintf.h"
#include "device.h"


/*
  メモリダンプ
*/
static unsigned char *mdAddr;	// ダンプアドレス
static unsigned char mdDs;	// ダンプメモリサイズ 1:char, 2:short, 4:long

static void init_dump(void)
{
#ifdef GSC_TARGET_SYSTEM_EMU
  #ifdef GSC_MEMORY_HEAP_IS_NEWLIB
	mdAddr = 0;
  #else
	mdAddr = (unsigned char *)MEM_START;
  #endif
#else
	mdAddr = 0;
#endif
	mdDs = 1;
}

static int dump(int argc, uchar *argv[]);

/**
   @brief	メモリダンプ結果を表示する
*/
static const struct st_shell_command com_mem_dump = {
	.name		= "dump",
	.init		= init_dump,
	.command	= dump,
	.attr		= CMDATTR_CTAL,
	.usage_str	= "[-b|w|l] [start [end]]",
	.manual_str	= "Dump memory"
};

static int dump(int argc, uchar *argv[])
{
	unsigned char *ed = mdAddr + 256;
	unsigned char *dp;
	int i, an = 0;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'b': mdDs = 1; break;
			case 'w': mdDs = 2; break;
			case 'l': mdDs = 4; break;
			default:
				break;
			}
		} else {
			switch(an) {
			case 0:
				mdAddr	= (unsigned char *)(long)hstoi(argv[i]);
				ed		= mdAddr + 256;
				break;

			case 1:
				ed	= (unsigned char *)(long)hstoi(argv[i]);
				break;

			default:
				break;
			}
			an ++;
		}
	}

	for(dp=mdAddr; dp<ed; dp+=16) {
		unsigned char *p;
		uchar rd;

		tprintf("%p : ", dp);

		switch(mdDs) {
		case 1:
			for(p=dp; p<dp+8; p++) {
				if(p < ed) {
					tprintf("%02X ", (int)*p);
				} else {
					tprintf("   ");
				}
			}
			tprintf(" ");
			for(p=dp+8; p<dp+16; p++) {
				if(p < ed) {
					tprintf("%02X ", (int)*p);
				} else {
					tprintf("   ");
				}
			}
			break;
		case 2:
			for(p=dp; p<dp+16; p+=2) {
				if(p < ed) {
					tprintf("%04X ", *(unsigned short *)p);
				} else {
					tprintf("   ");
				}
			}
			break;

		case 4:
			for(p=dp; p<dp+16; p+=4) {
				if(p < ed) {
					tprintf("%08lX ", *(unsigned long *)p);
				} else {
					tprintf("   ");
				}
			}
			break;

		default:
			break;
		}

		tprintf("  \"");

		for(p=dp; p<dp+16; p++) {
			if(p < ed) {
				if(((' ' <= *p) && (*p <= 'Z'))
				   || (('a' <= *p) && (*p <= 'z'))) {
					cputc(*p);
				} else {
					cputc('.');
				}
			} else {
				break;
			}
		}
		tprintf("\"\n");

		if(cgetcnw(&rd) == 0) {
			if(rd == ASCII_CTRL_D) {
				tprintf("Abort.\n");
				return 0;
			}
		}
	}

	mdAddr = ed;

	return 0;
}


static int fill(int argc, uchar *argv[]);

/**
   @brief	任意のメモリ範囲にメモリを任意の値を書き込む
*/
static const struct st_shell_command com_mem_fill = {
	.name		= "fill",
	.command	= fill,
	.usage_str	= "<start> <end> <val>",
	.manual_str	= "Fill memory"
};

static int fill(int argc, uchar *argv[])
{
	unsigned char ds = 1;	// 1:char, 2:short, 4:long
	unsigned char *st = 0, *ed = 0;
	unsigned char *dp;
	unsigned int dt = 0;
	int i, an = 0;

	for(i=1; i<argc; i++) {
		if(argv[i][0] == '-') {
			switch(argv[i][1]) {
			case 'b': ds = 1; break;
			case 'w': ds = 2; break;
			case 'l': ds = 4; break;
			default:
				break;
			}
		} else {
			switch(an) {
			case 0:
				st = (unsigned char *)(long)hstou(argv[i]);
				an++;
				break;

			case 1:
				ed = (unsigned char *)(long)hstou(argv[i]);
				an++;
				break;

			case 2:
				dt = (unsigned int)hstoi(argv[i]);
				an++;
				break;

			default:
				break;
			}
		}
	}

	if(an < 3) {
		print_command_usage(&com_mem_fill);
		return 0;
	}

	tprintf("Fill memory %p - %p ", st, ed);

	switch(ds) {
	case 1:
		tprintf("%02X", dt);
		break;

	case 2:
		tprintf("%04X", dt);
		break;

	case 4:
		tprintf("%08X", dt);
		break;

	default:
		break;
	}

	tprintf("\n");

	for(dp=st; dp<=ed; dp+=ds) {
		switch(ds) {
		case 1:
			*dp = dt;
			break;

		case 2:
			*(unsigned short *)dp = (unsigned short)dt;
			break;

		case 4:
			*(unsigned long *)dp = (unsigned long)dt;
			break;

		default:
			break;
		}
	}

	return 0;
}


/*
  メモリ編集
*/
static void *meAddr;	// 編集アドレス

static void init_memedit(void)
{
#ifdef GSC_TARGET_SYSTEM_EMU
  #ifdef GSC_MEMORY_HEAP_IS_NEWLIB
	meAddr = 0;
  #else
	meAddr = MEM_START;
  #endif
#else
	meAddr = 0;
#endif
}

static int memedit(int argc, uchar *argv[])
{
	static int wi = 0;	// 0:byte 1:word 2:long
	unsigned char an = 1;

	switch(argv[0][1]) {
	case 'b':
		wi = 0;
		break;

	case 'w':
		wi = 1;
		break;

	case 'l':
		wi = 2;
		break;
	}

	if(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'b':
			wi = 0;
			break;

		case 'w':
			wi = 1;
			break;

		case 'l':
			wi = 2;
			break;
		}

		an ++;
	}

	if(argc > an) {
		meAddr = (void *)(unsigned long)hstou(argv[an]);
		an ++;
	}

	if(argc > an) {
		// メモリ書き込み値あり

		int i;

		for(i=an; i<argc; i++) {

			switch(wi) {
			case 0:
				*(unsigned char *)meAddr =
						(unsigned char)hstoi(argv[i]);
				meAddr ++;
				break;

			case 1:
				*(unsigned short *)meAddr =
						(unsigned short)hstoi(argv[i]);
				meAddr += 2;
				break;

			case 2:
				*(unsigned long *)meAddr =
						(unsigned long)hstoi(argv[i]);
				meAddr += 4;
				break;
			}
		}
	} else {
		// メモリ書き込み値無し

		tprintf("%p : ", meAddr);

		switch(wi) {
		case 0:
			tprintf("%02X", (int)*(unsigned char *)meAddr);
			meAddr ++;
			break;

		case 1:
			tprintf("%04X", (int)*(unsigned short *)meAddr);
			meAddr += 2;
			break;

		case 2:
			tprintf("%08lX", (long)*(unsigned long *)meAddr);
			meAddr += 4;
			break;
		}

		tprintf("\n");
	}

	return 0;
}

/**
   @brief	メモリを編集する
*/
static const struct st_shell_command com_mem_edit = {
	.name		= "edit",
	.init		= init_memedit,
	.command	= memedit,
	.attr		= CMDATTR_CTAL,
	.usage_str	= "[-b|w|l] [address] [value ...]",
	.manual_str	= "Edit memory"
};

static const struct st_shell_command com_memedit_b = {
	.name		= "mb",
	.command	= memedit,
	.attr		= CMDATTR_CTAL,
	.usage_str	= "[address] [value ...]",
	.manual_str	= "Byte edit memory"
};

static const struct st_shell_command com_memedit_w = {
	.name		= "mw",
	.command	= memedit,
	.attr		= CMDATTR_CTAL,
	.usage_str	= "[address] [value ...]",
	.manual_str	= "Word edit memory"
};

static const struct st_shell_command com_memedit_l = {
	.name		= "ml",
	.command	= memedit,
	.attr		= CMDATTR_CTAL,
	.usage_str	= "[address] [value ...]",
	.manual_str	= "Long word edit memory"
};


/*
  メモリチェック
*/

static int memory_check(unsigned long *sp, unsigned long *ep,
			unsigned long data)
{
	unsigned long *dp;
	uchar rd;
	int err = 0;

	tprintf("Writing %08lX...\n", data);

	for(dp=sp; dp<=ep; dp++) {
		*dp = data;
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	tprintf("Checking...");
	for(dp=sp; dp<=ep; dp++) {
		if(*dp != data) {
			err = 1;
			tprintf("\nVerify Error at %p W:%08lX R:%08lX",
				dp, data, *dp);
		}
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	if(err) {
		tprintf("\nNG.\n");
	} else {
		tprintf("OK.\n");
	}

	return 0;
}

static int memory_check_a(unsigned long *sp, unsigned long *ep)
{
	unsigned long *dp;
	uchar rd;
	int err = 0;

	tprintf("Writing address data...\n");

	for(dp=sp; dp<=ep; dp++) {
		*dp = (unsigned long)dp;
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	tprintf("Checking...");
	for(dp=sp; dp<=ep; dp++) {
		if(*dp != (unsigned long)dp) {
			err = 1;
			tprintf("\nVerify Error at %p W:%08lX R:%08lX", dp,
				(unsigned long)dp,
				(unsigned long)*dp);
		}
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	if(err) {
		tprintf("\nNG.\n");
	} else {
		tprintf("OK.\n");
	}

	return 0;
}

static int memory_check_ar(unsigned long *sp, unsigned long *ep)
{
	unsigned long *dp;
	uchar rd;
	int err = 0;

	tprintf("Writing invert address data...\n");

	for(dp=sp; dp<=ep; dp++) {
		*dp = ~((unsigned long)dp);
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	tprintf("Checking...");
	for(dp=sp; dp<=ep; dp++) {
		if(*dp != ~((unsigned long)dp)) {
			err = 1;
			tprintf("\nVerify Error at %p W:%08lX R:%08lX",
				dp,
				~((unsigned long)dp),
				(unsigned long)*dp);
		}
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	if(err) {
		tprintf("\nNG.\n");
	} else {
		tprintf("OK.\n");
	}

	return 0;
}

static int memory_check_aq(unsigned long *sp, unsigned long *ep)
{
	unsigned long *dp;
	uchar rd;
	int err = 0;

	tprintf("Write and checking address data...\n");
	tprintf("Checking...");

	for(dp=sp; dp<=ep; dp++) {
		*dp = (unsigned long)dp;
		if(*dp != (unsigned long)dp) {
			err = 1;
			tprintf("\nVerify Error at %p W:%08lX R:%08lX",
				dp,
				(unsigned long)dp,
				(unsigned long)*dp);
		}
		if(cgetcnw(&rd)) {
			if(rd == ASCII_CTRL_C) {
				tprintf("\nAbort.\n");
				return 1;
			}
		}
	}

	if(err) {
		tprintf("\nNG.\n");
	} else {
		tprintf("OK.\n");
	}

	return 0;
}


static int com_memchk(int argc, uchar *argv[]);

/**
   @brief	メモリのチェックを行う
*/
static const struct st_shell_command com_mem_check = {
	.name		= "check",
	.command	= com_memchk,
	.usage_str	= "[Start [End]]",
	.manual_str	= "Memory check"
};

/**
   @brief	メモリチェック
   @param[in]	argc	コマンド引数の数
   @param[in]	argv	コマンド引数文字列のポインタ
   @return	コマンド実行結果
*/
static int com_memchk(int argc, uchar *argv[])
{
#ifdef GSC_TARGET_SYSTEM_EMU
  #ifdef GSC_MEMORY_HEAP_IS_NEWLIB
	unsigned long *sp = 0;
	unsigned long *ep = 0;
  #else
	unsigned long *sp = (unsigned long *)MEM_START;
	unsigned long *ep = (unsigned long *)MEM_END;
  #endif
#else
	unsigned long *sp = 0;
	unsigned long *ep = 0;
#endif

	if(argc > 1) {
		sp = (unsigned long *)(long)hstoi(argv[1]);
	}

	if(argc > 2) {
		ep = (unsigned long *)(long)hstoi(argv[2]);
	}

	tprintf("Memory check.\n");
	tprintf("Start  : %p\n", sp);
	tprintf("End    : %p\n", ep);

	if(memory_check(sp, ep, 0x00000000)) goto end;
	if(memory_check(sp, ep, 0xffffffff)) goto end;
	if(memory_check(sp, ep, 0xaaaaaaaa)) goto end;
	if(memory_check(sp, ep, 0x55555555)) goto end;
	if(memory_check_a(sp, ep)) goto end;
	if(memory_check_ar(sp, ep)) goto end;
	if(memory_check_aq(sp, ep)) goto end;
	if(memory_check(sp, ep, 0x55aa55aa)) goto end;
	if(memory_check(sp, ep, 0x00ff00ff)) goto end;
	if(memory_check(sp, ep, 0x00000000)) goto end;

end:

	return 0;
}

static const struct st_shell_command * const com_mem_list[] = {
	&com_mem_dump,
	&com_mem_fill,
	&com_mem_edit,
	&com_memedit_b,
	&com_memedit_w,
	&com_memedit_l,
	&com_mem_check,
	0
};

const struct st_shell_command com_mem = {
	.name		= "mem",
	.manual_str	= "Memory operation commands",
	.sublist	= com_mem_list
}; ///< メモリ表示、編集

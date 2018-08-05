/** @file
    @brief	機能限定printf

    @date	2011.02.03
    @date	2007.03.10
    @date	2002.03.02

    @author	Takashi SHUDO
*/

#include "vtprintf.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "str.h"
#include "console.h"

#if ARCH != cortex-m7 // ARM GCC or NEWLIB Bug ?
#define LONGLONGSWAP
#endif

static int print_write(io_write write, unsigned int *len, unsigned int size, uchar *str, unsigned int slen)
{
	if((size != 0) && (slen > (size - *len))) {
		slen = size - *len;
	}

	write(str, (unsigned int)slen);

	*len += slen;

	if((size != 0) && (*len >= size)) {
		return 1;
	}

	return 0;
}

/**
   @brief	簡易printf、float,doubleは使えない

   %ns,%nd,%nx,%nXのみ<br>
   %[n]s	- 文字列表示<br>
	  [n] 省略時:文字列長<br>
   %[n]d	- 10進数表示<br>
	  [n] 省略時:デコード文字列長<br>
	  %の次が"0"ならば"0"付き表示<br>
   %[n]X - 16進数表示(大文字)<br>
   %[n]x	- 16進数表示(小文字)(必ず"0"付きで桁数分表示)<br>
	  [n] 0-8 省略時:桁数分<br>
   %c	- 文字表示<br>
   %p	- アドレス表示(32bit:XXXXXXXX, 64bit:XXXXXXXXXXXXXXXX<br>

   @param[in]	dev	出力デバイス
   @param[in]	fmt	フォーマット文字列
   @param[in]	args	引数リスト

   @return	出力文字数
*/
int vtprintf(io_write write, const char *fmt, unsigned int size, va_list args)
{
	uchar str[MAXFORMATSTR];	// フォーマットデコードバッファ
	int dec = 0;		// 1:%デコード中
	unsigned int strlen = 0;		// 桁数
	int zero = 0;		// "0"付きか 0:不明,1:"0"付き,-1:"0"無し
	int flong = 0;		// 0:int, 1:long, 2:long long
	unsigned int len = 0;
	unsigned int slen;

	if(write == 0) {
		SYSERR_PRINT("write = 0\n");
	}

	while(*fmt != 0) {
	    if(dec == 0) {
		if(*fmt == '%') {
			strlen = 0;
			zero = 0;
			dec = 1;
			flong = 0;
		} else {
			if(*fmt == '\n') {
				if(print_write(write, &len, size, (uchar *)"\r", 1) != 0) {
					return (int)len;
				}
			}
			if(print_write(write, &len, size, (uchar *)fmt, 1) != 0) {
				return (int)len;
			}
		}
	    } else {
		if(('0' <= *fmt) && (*fmt <= '9')) {
			if(zero == 0) {
				if(*fmt == '0') {
					zero = 1;
				} else {
					zero = -1;
					strlen = (strlen*10) + *fmt - '0';
				}
			} else {
				strlen = (strlen*10) + *fmt - '0';
			}
		} else {
			switch(*fmt) {
			case '%':
				if(print_write(write, &len, size, (uchar *)"%", 1) != 0) {
					return (int)len;
				}
				dec = 0;
				break;

			case 'l':
				if(flong == 0) {
					flong = 1;
				} else {
					flong = 2;
				}
				break;

			case 's':
				if(strlen) {
					unsigned int i;
					uchar *p = va_arg(args, uchar *);
					for(i=0; i<strlen; i++) {
						if((*p) != 0) {
							if(print_write(write, &len, size, p, 1) != 0) {
								return (int)len;
							}
							p++;
						} else {
							if(print_write(write, &len, size, (uchar *)" ", 1) != 0) {
								return (int)len;
							}
						}
					}
				} else {
					uchar *p = va_arg(args, uchar *);
					slen = strleng(p);
					if((size != 0) && (slen > (size - len))) {
						slen = size - len;
					}
					if(print_write(write, &len, size, p, slen) != 0) {
						return (int)len;
					}
				}
				dec = 0;
				break;

			case 'i':
			case 'd':
				if(strlen == 0) {
					uchar *p;
					if(flong == 1) {
						long val = va_arg(args, long);
						(void)itods(str, MAXFORMATSTR-1, val);
					} else if(flong == 2) {
						long long val = va_arg(args, long long);
#ifdef LONGLONGSWAP
						val = (val >> 32) | (val << 32);
#endif
						(void)lltods(str, MAXFORMATSTR-1, val);
					} else {
						int val = va_arg(args, int);
						(void)itods(str, MAXFORMATSTR-1, val);
					}
					for(p=str;
					    p<&str[MAXFORMATSTR];
					    p++) {
						    if(*p != ' ') {
							    slen = strleng((uchar *)p);
							    if((size != 0) && (slen > (size - len))) {
								    slen = size - len;
							    }
							    if(print_write(write, &len, size, p, slen) != 0) {
								    return (int)len;
							    }
							    break;
						    }
					}
				} else {
					if(zero == 1) {
						if(flong == 1) {
							long val = va_arg(args, long);
							(void)itodsz(str, strlen, val);
						} else if(flong == 2) {
							long long val = va_arg(args, long long);
#ifdef LONGLONGSWAP
							val = (val >> 32) | (val << 32);
#endif
							(void)lltodsz(str, strlen, val);
						} else {
							int val = va_arg(args, int);
							(void)itodsz(str, strlen, val);
						}
					} else {
						if(flong == 1) {
							long val = va_arg(args, long);
							(void)itods(str, strlen, val);
						} else if(flong == 2) {
							long long val = va_arg(args, long long);
#ifdef LONGLONGSWAP
							val = (val >> 32) | (val << 32);
#endif
							(void)lltods(str, strlen, val);
						} else {
							int val = va_arg(args, int);
							(void)itods(str, strlen, val);
						}
					}
					slen = strleng(str);
					if((size != 0) && (slen > (size - len))) {
						slen = size - len;
					}
					if(print_write(write, &len, size, str, slen) != 0) {
						return (int)len;
					}
				}
				dec = 0;
				break;

			case 'u':
				if(strlen == 0) {
					uchar *p;
					if(flong == 1) {
						unsigned long val = va_arg(args, unsigned long);
						(void)uitods(str, MAXFORMATSTR-1, (unsigned int)val);
					} else if(flong == 2) {
						unsigned long long val = va_arg(args, unsigned long long);
#ifdef LONGLONGSWAP
						val = (val >> 32) | (val << 32);
#endif
						(void)ulltods(str, MAXFORMATSTR-1, val);
					} else {
						unsigned int val = va_arg(args, unsigned int);
						(void)uitods(str, MAXFORMATSTR-1, val);
					}
					for(p=str;
					    p<&str[MAXFORMATSTR];
					    p++) {
						    if(*p != ' ') {
							    slen = strleng(p);
							    if((size != 0) && (slen > (size - len))) {
								    slen = size - len;
							    }
							    if(print_write(write, &len, size, p, slen) != 0) {
								    return (int)len;
							    }
							    dec = 0;
							    break;
						    }
					}
				} else {
					if(zero == 1) {
						if(flong == 1) {
							unsigned long val = va_arg(args, unsigned long);
							(void)uitodsz(str, strlen, (unsigned int)val);
						} else if(flong == 2) {
							unsigned long long val = va_arg(args, unsigned long long);
#ifdef LONGLONGSWAP
							val = (val >> 32) | (val << 32);
#endif
							(void)ulltodsz(str, strlen, val);
						} else {
							unsigned int val = va_arg(args, unsigned int);
							(void)uitodsz(str, strlen, val);
						}
					} else {
						if(flong == 1) {
							unsigned long val = va_arg(args, unsigned long);
							(void)uitods(str, strlen, (unsigned int)val);
						} else if(flong == 2) {
							unsigned long long val = va_arg(args, unsigned long long);
#ifdef LONGLONGSWAP
							val = (val >> 32) | (val << 32);
#endif
							(void)ulltods(str, strlen, val);
						} else {
							unsigned int val = va_arg(args, unsigned int);
							(void)uitods(str, strlen, val);
						}
					}
					slen = strleng(str);
					if((size != 0) && (slen > (size - len))) {
						slen = size - len;
					}
					if(print_write(write, &len, size, str, slen) != 0) {
						return (int)len;
					}
				}
				dec = 0;
				break;

			case 'x':
			case 'X':
				if(flong == 1) {
					long val = va_arg(args, long);
					if(strlen == 0) {
						strlen = (unsigned int)sizeof(long)*2;
					}
					(void)itohs(str, strlen, val);
				} else if(flong == 2) {
					long long val = va_arg(args, long long);
					if(strlen == 0) {
						strlen = (unsigned int)sizeof(long)*4;
					}
#ifdef LONGLONGSWAP
					val = (val >> 32) | (val << 32);
#endif
					(void)lltohs(str, strlen, val);
				} else {
					int val = va_arg(args, int);
					if(strlen == 0) {
						strlen = (unsigned int)sizeof(int)*2;
					}
					(void)itohs(str, strlen, val);
				}
				if(*fmt == 'X') {
					(void)str2cap(str);
				}
				slen = strleng(str);
				if((size != 0) && (slen > (size - len))) {
					slen = size - len;
				}
				if(print_write(write, &len, size, str, slen) != 0) {
					return (int)len;
				}
				dec = 0;
				break;

			case 'c':
				{
					uchar data[1];
					data[0] = (uchar)va_arg(args, int);
					if(print_write(write, &len, size, data, 1) != 0) {
						return (int)len;
					}
					dec = 0;
				}
				break;

			case 'p':
			case 'P':
#ifdef __x86_64__
				(void)lltohs(str, 16, va_arg(args, long long));
#else
				(void)itohs(str, 8, va_arg(args, int));
#endif
				if(*fmt == 'P') {
					(void)str2cap(str);
				}
				slen = 8;
				if((size != 0) && (slen > (size - len))) {
					slen = size - len;
				}
#ifdef __x86_64__
				if(print_write(write, &len, size, str+8, slen) != 0) {
					return (int)len;
				}
#else
				if(print_write(write, &len, size, str, slen) != 0) {
					return (int)len;
				}
#endif
				dec = 0;
				break;

			default:
				break;
			}
		}
	    }

	    fmt ++;
	}

	return (int)len;
}


void vxdump(unsigned int addr, unsigned char *data, unsigned int len,
	    int addr_type,	/* XDUMP_ADDR_* */
	    int data_size,	/* XDUMP_DATA_*	*/
	    int(* print)(const char *fmt, ...))
{
	int i = 0, j;
	uchar cbuf[16];

	switch(addr_type) {
	case XDUMP_ADDR_ANY_WORD:
		print("%04X : ", addr + i);
		break;

	case XDUMP_ADDR_ANY_LONG:
		print("%08X : ", addr + i);
		break;

	case XDUMP_ADDR_DATA_ADDR:
		print("%p : ", data);
		break;

	default:
		print("%p : ", data);
		break;
	}

	while(len) {
		cbuf[i % 16] = *data;
		switch(data_size) {
		case 0:
			print("%02X ", *data);
			break;

		case XDUMP_DATA_WORD:
#ifdef __x86_64__
			if(((unsigned long long)data % 2) == 0) {
#else
			if(((unsigned long)data % 2) == 0) {
#endif
				print("%04lX ", *(unsigned long *)data);
			}
			break;

		case XDUMP_DATA_LONG:
#ifdef __x86_64__
			if(((unsigned long long)data % 4) == 0) {
#else
			if(((unsigned long)data % 4) == 0) {
#endif
				print("%08lX ", *(unsigned long *)data);
			}
			break;

		default:
			print("%02X ", *data);
			break;
		}
		data ++;
		len --;
		i ++;
		if((i % 8) == 0) {
			print(" ");
		}
		if((i % 16) == 0) {
			print(" \"");
			for(j=0; j<16; j++) {
				if((' ' <= cbuf[j]) && (cbuf[j] <= '~')) {
					print("%c", cbuf[j]);
				} else {
					print(".");
				}
			}
			print("\"");
			if(len != 0) {
				switch(addr_type) {
				case XDUMP_ADDR_ANY_WORD:
					print("\n%04X : ", addr + i);
					break;

				case XDUMP_ADDR_ANY_LONG:
					print("\n%08X : ", addr + i);
					break;

				case XDUMP_ADDR_DATA_ADDR:
					print("\n%p : ", data);
					break;

				default:
					break;
				}
			} else {
				print("\n");
			}
		} else {
			if(len == 0) {
				if((i % 16) < 8) {
					print("  ");
				} else {
					print(" ");
				}
				for(j=0; j<(16-(i % 16)); j++) {
					print("   ");
				}
				print(" \"");
				for(j=0; j<(i % 16); j++) {
					if((' ' <= cbuf[j]) && (cbuf[j] <= '~')) {
						print("%c", cbuf[j]);
					} else {
						print(".");
					}
				}
				print("\"\n");
			}
		}
	}
}

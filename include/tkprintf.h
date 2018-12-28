/** @file
    @brief	カーネル用機能限定printf

    @date	2017.08.28
    @author	Takashi SHUDO
*/

#ifndef TKPRINTF_H
#define TKPRINTF_H

#include "sysconfig.h"
#include "device.h"

int register_kmess_out_dev(struct st_device *dev);
unsigned int set_kernel_message_out_mem(unsigned char *mp, unsigned int size);
int kputs(unsigned char *str, unsigned int len);
int tkprintf(const char *fmt, ...)__attribute__ ((format(printf, 1, 2)));
void kxdump(unsigned char *data, unsigned int len);
void kxbdump(unsigned char *data, unsigned int len);

#define SYSERR_COLOR	"\033[31m"	// RED
#define NORMAL_COLOR	"\033[m"	// default color

#if 1
#ifndef LINT
#define SYSERR_PRINT(format, arg...)	tkprintf(SYSERR_COLOR __FILE__	\
						 ":%d %s: "		\
						 format NORMAL_COLOR,	\
						 (int)__LINE__,		\
						 __FUNCTION__,		\
						 ##arg)
#else
#define SYSERR_PRINT(format, arg...)
#endif
#else
#define SYSERR_PRINT(format, arg...)	tkprintf(__FILE__ \
						 ":%d " format,		\
						 (int)__LINE__, ##arg)
#endif

#endif // TKPRINTF_H

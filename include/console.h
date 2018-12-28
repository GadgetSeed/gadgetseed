/** @file
    @brief	コンソールI/O

    @date	2007.03.17
    @author	Takashi SHUDO
*/

#ifndef CONSOLE_H
#define CONSOLE_H

#include "sysconfig.h"
#include "device.h"

typedef int (* io_write)(unsigned char *data, unsigned int count);
typedef int (* io_read)(unsigned char *data, unsigned int count);

extern void register_console_in_dev(const struct st_device *in_dev);
extern void register_console_out_dev(const struct st_device *out_dev);
extern void init_console_device(void);

extern int cputs(unsigned char *str, unsigned int count);
extern int cgets(unsigned char *str, unsigned int count);
extern int cputc(unsigned char td);
extern int cgetc(unsigned char *rd);
extern int cwait(unsigned int timeout);
extern int cgetcnw(unsigned char *rd);

extern void register_error_out_dev(const struct st_device *err_dev);
extern int eputs(unsigned char *str, unsigned int len);

#ifdef GSC_KERNEL_MESSAGEOUT_LOG
extern void register_log_out_dev(const struct st_device *err_dev);
extern int lputs(unsigned char *str, unsigned int len);
#endif

extern void set_console_in_device_ISR(struct st_device *dev);
extern void set_console_out_device_ISR(struct st_device *dev);
extern void set_error_out_device_ISR(struct st_device *dev);

#endif	// CONSOLE_H

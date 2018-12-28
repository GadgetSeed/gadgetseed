/** @file
    @brief	システムログ

    @date	2018.12.09
    @author	Takashi SHUDO
*/

#ifndef LOG_H
#define LOG_H

#include "device.h"

int register_kmess_log_dev(struct st_device *dev);

void set_logtimestamp_display(int flg);
int get_logtimestamp_display(void);
void set_log_record_priority(int pri);
int get_log_record_priority(void);
void set_log_display_priority(int pri);
int get_log_display_priority(void);

int gslog(int pri, const char *fmt, ...)__attribute__ ((format(printf, 2, 3)));

#ifndef GSLOG_PREFIX
#define GSLOG_PREFIX	""
#endif
#define GSLOG(pri, fmt, arg...)	gslog(pri, GSLOG_PREFIX fmt, ##arg)

#ifdef GSC_KERNEL_MESSAGEOUT_LOG	// $gsc カーネルメッセージのログ出力を有効にする
int gslogn(const char *fmt, ...)__attribute__ ((format(printf, 1, 2)));
void lxdump(int pri, unsigned char *data, unsigned int len);
#else
#define gslogn(fmt)
#define lxdump(pri,data,len)
#endif
#endif // LOG_H

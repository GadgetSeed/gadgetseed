/** @file
    @brief	システムログ

    @date	2018.12.09
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "log.h"
#include "console.h"
#include "vtprintf.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "str.h"
#include "timer.h"

extern io_write kmess_out_func;

struct st_device *kmess_log_dev;
io_write kmess_log_func;
int flg_disp_logtimestamp = 0;

#ifndef GSC_LOG_RECORDLOGPRI
#define GSC_LOG_RECORDLOGPRI	8	// $gsc 記録するログ優先順位
#endif

#ifndef GSC_LOG_DISPLOGPRI
#define GSC_LOG_DISPLOGPRI	1	// $gsc 表示するログ優先順位
#endif

static int record_log_pri = GSC_LOG_RECORDLOGPRI;
static int disp_log_pri = GSC_LOG_DISPLOGPRI;
static int flg_log_cr = 1;


static int kputs_log(unsigned char *str, unsigned int len)
{
#if 0
	return write_device(kmess_dev, str, len);
#else // LF = CRLF
	int i;

	for(i=0; i<len; i++) {
		if(*str == '\n') {
			putc_device(kmess_log_dev, '\r');
		}
		putc_device(kmess_log_dev, *str);
		str ++;
	}

	return i;
#endif
}

/**
   @brief	カーネルメッセージロギングデバイスを登録する

   @param[in]	dev	カーネルメッセージロギングデバイス

   @return	0固定
*/
int register_kmess_log_dev(struct st_device *dev)
{
	kmess_log_dev = dev;
	kmess_log_func = kputs_log;

	return 0;
}


/**
   @brief	tkprintfのタイムスタンプ付与を設定する

   @param[in]	flg	0:非表示、!=0:表示有効
*/
void set_logtimestamp_display(int flg)
{
	flg_disp_logtimestamp = flg;
}

/**
   @brief	tkprintfのタイムスタンプ付与を設定を取得する

   @param[in]	flg	0:非表示、!=0:表示有効
*/
int get_logtimestamp_display(void)
{
	return flg_disp_logtimestamp;
}

/**
   @brief	ログの記録を行う優先度を設定する

   @param[in]	pri	表示優先度
*/
void set_log_record_priority(int pri)
{
	record_log_pri = pri;
}

/**
   @brief	ログの記録を行う優先度を取得する

   @return	表示優先度
*/
int get_log_record_priority(void)
{
	return record_log_pri;
}

/**
   @brief	ログのリアルタイム表示を行う優先度を設定する

   @param[in]	pri	表示優先度
*/
void set_log_display_priority(int pri)
{
	disp_log_pri = pri;
}

/**
   @brief	ログのリアルタイム表示を行う優先度を取得する

   @return	表示優先度
*/
int get_log_display_priority(void)
{
	return disp_log_pri;
}


/*
 *
 */

/**
   @brief	ログメッセージ記録

   @param[in]	pri	優先順位
   @param[in]	fmt	メッセージフォーマット

   @return	出力メッセージサイズ
*/
int gslog(int pri, const char *fmt, ...)
{
	va_list	args;
	int len = 0;
	char str[MAXFORMATSTR];	// フォーマットデコードバッファ
	int pslen = 0;

	if(flg_log_cr != 0) {
		if(pri <= record_log_pri) {
			len = str_timestamp(str);
			if((flg_disp_logtimestamp != 0) && (pri <= disp_log_pri)) {
				kmess_out_func((unsigned char *)str, len);
			} else {
				kmess_log_func((unsigned char *)str, len);
			}
			pslen = tsnprintf(str, MAXFORMATSTR, "(%d) ", pri);
			if((flg_disp_logtimestamp != 0) && (pri <= disp_log_pri)) {
				kmess_out_func((unsigned char *)str, pslen);
			} else {
				kmess_log_func((unsigned char *)str, pslen);
			}
			len += pslen;
		}
	}

	if(fmt[strleng((const uchar *)fmt)-1] == '\n') {
		flg_log_cr = 1;
	} else {
		flg_log_cr = 0;
	}

	va_start(args, fmt);
	if(pri <= record_log_pri) {
		if(pri <= disp_log_pri) {
			len += vtprintf(kmess_out_func, fmt, 0, args);
		} else {
			len += vtprintf(kmess_log_func, fmt, 0, args);
		}
	}
	va_end(args);

	return len;
}

/**
   @brief	ログメッセージ記録

   @param[in]	fmt	メッセージフォーマット

   @return	出力メッセージサイズ
*/
#ifndef GSC_DEFAULT_LOGPRIORITY
#define GSC_DEFAULT_LOGPRIORITY	5	///< $gsc gslogn()のログプライオリティ
#endif

int gslogn(const char *fmt, ...)
{
	va_list	args;
	int len = 0;
	char str[MAXFORMATSTR];	// フォーマットデコードバッファ
	int pslen = 0;

	if(flg_log_cr != 0) {
		len = str_timestamp(str);
		if(flg_disp_logtimestamp != 0) {
			kmess_out_func((unsigned char *)str, len);
		} else {
			kmess_log_func((unsigned char *)str, len);
		}
		pslen = tsnprintf(str, MAXFORMATSTR, "(%d) ", GSC_DEFAULT_LOGPRIORITY);
		if(flg_disp_logtimestamp != 0) {
			kmess_out_func((unsigned char *)str, pslen);
		} else {
			kmess_log_func((unsigned char *)str, pslen);
		}
		len += pslen;
	}

	if(fmt[strleng((const uchar *)fmt)-1] == '\n') {
		flg_log_cr = 1;
	} else {
		flg_log_cr = 0;
	}

	va_start(args, fmt);
	if(GSC_DEFAULT_LOGPRIORITY < disp_log_pri) {
		len += vtprintf(kmess_out_func, fmt, 0, args);
	} else {
		len += vtprintf(kmess_log_func, fmt, 0, args);
	}
	va_end(args);

	return len;
}

void lxdump(int pri, unsigned char *data, unsigned int len)
{
	if(pri <= record_log_pri) {
		if(pri <= disp_log_pri) {
			xdump(data, len);
		}
	}
}

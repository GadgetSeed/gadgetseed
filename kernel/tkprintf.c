/** @file
    @brief	カーネル用機能限定printf

    @date	2017.08.28
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "vtprintf.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "timer.h"

static struct st_device *kmess_dev;
io_write kmess_out_func;

#ifdef GSC_KERNEL_MESSAGEOUT_LOG
extern struct st_device *kmess_log_dev;
extern io_write kmess_log_func;
extern int flg_disp_logtimestamp;
#endif

static unsigned char *kmess_sp;
static unsigned int kmess_size, kmess_count;

static int ksputs(unsigned char *str, unsigned int len)
{
	unsigned int i;
	int count = 0;

	for(i=0; i<len; i++) {
		if(kmess_count >= kmess_size) {
			*(kmess_sp - 1) = '!'; // バッファ終端にたどり着いた
			break;
		}
		*kmess_sp = *str;
		kmess_sp ++;
		str ++;
		kmess_count ++;
		count ++;
	}

	return count;
}

int kputs(unsigned char *str, unsigned int len)
{
#if 0
	return write_device(kmess_dev, str, len);
#else // LF = CRLF
	int i;

	for(i=0; i<len; i++) {
		if(*str == '\n') {
			putc_device(kmess_dev, '\r');
		}
		putc_device(kmess_dev, *str);
		str ++;
	}

	return i;
#endif
}

/**
   @brief	カーネルメッセージ出力デバイスを登録する

   @param[in]	dev	カーネルメッセージ出力デバイス

   @return	0固定
*/
int register_kmess_out_dev(struct st_device *dev)
{
	kmess_dev = dev;
	kmess_out_func = kputs;

	return 0;
}

/**
   @brief	カーネルメッセージ出力メモリアドレスを設定する

   @param[in]	mp	カーネルメッセージ出力メモリアドレス
   @param[in]	size	カーネルメッセージ出力メモリサイズ

   @return	カーネルメッセージ出力メモリサイズ
*/
unsigned int set_kernel_message_out_mem(unsigned char *mp, unsigned int size)
{
	if(mp == 0) {
		kmess_out_func = kputs;
	} else {
		kmess_sp = mp;
		kmess_size = size; 
		kmess_count = 0; 
		kmess_out_func = ksputs;
	}

	return kmess_count;
}

/**
   @brief	非タスクコンテキスト実行用メッセージ出力

   @param[in]	fmt	メッセージフォーマット

   @return	出力メッセージサイズ
*/
int tkprintf(const char *fmt, ...)
{
	va_list	args;
	int len = 0;

#ifdef GSC_KERNEL_MESSAGEOUT_LOG
	char str[MAXFORMATSTR];	// フォーマットデコードバッファ

	len = str_timestamp(str);
	if(flg_disp_logtimestamp != 0) {
		kmess_out_func((unsigned char *)str, len);
		kmess_out_func((unsigned char *)"(K) ", 4);
	} else {
		kmess_log_func((unsigned char *)str, len);
		kmess_log_func((unsigned char *)"(K) ", 4);
	}
#endif

	va_start(args, fmt);
	len += vtprintf(kmess_out_func, fmt, 0, args);
	va_end(args);

	return len;
}

#ifndef GSC_KERNEL_MESSAGEOUT_LOG
#include "log.h"

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

	if(pri > 0) {
		return 0;
	}

	va_start(args, fmt);
	len += vtprintf(kmess_out_func, fmt, 0, args);
	va_end(args);

	return len;
}
#endif


/**
   @brief	非タスクコンテキスト実行用メモリダンプメッセージ出力

   @param[in]	data	ダンプ出力メモリ
   @param[in]	len	ダンプ出力メモリサイズ
*/
void kxdump(unsigned char *data, unsigned int len)
{
	vxdump(0, data, len, XDUMP_ADDR_DATA_ADDR, XDUMP_DATA_LONG, tkprintf);
}

/**
   @brief	非タスクコンテキスト実行用メモリダンプメッセージ出力

   @param[in]	data	ダンプ出力メモリ
   @param[in]	len	ダンプ出力メモリサイズ
*/
void kxbdump(unsigned char *data, unsigned int len)
{
	vxdump(0, data, len, XDUMP_ADDR_ANY_WORD, XDUMP_DATA_BYTE, tkprintf);
}

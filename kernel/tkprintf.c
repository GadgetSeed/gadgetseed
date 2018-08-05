/** @file
    @brief	カーネル用機能限定printf

    @date	2017.08.28
    @author	Takashi SHUDO
*/

#include "vtprintf.h"
#include "tkprintf.h"

static struct st_device *kmess_dev;
static io_write kmess_out;

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

static int kputs(unsigned char *str, unsigned int len)
{
	return write_device(kmess_dev, str, len);
}

/**
   @brief	カーネルメッセージ出力デバイスを登録する

   @param[in]	dev	カーネルメッセージ出力デバイス

   @return	0固定
*/
int register_kmess_out_dev(struct st_device *dev)
{
	kmess_dev = dev;
	kmess_out = kputs;

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
		kmess_out = kputs;
	} else {
		kmess_sp = mp;
		kmess_size = size; 
		kmess_count = 0; 
		kmess_out = ksputs;
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
	
	va_start(args, fmt);	
	len += vtprintf(kmess_out, fmt, 0, args);
	va_end(args);

	return len;
}

/**
   @brief	非タスクコンテキスト実行用メモリダンプメッセージ出力

   @param[in]	data	ダンプ出力メモリ
   @param[in]	len	ダンプ出力メモリサイズ
*/
void kxdump(unsigned char *data, unsigned int len)
{
	vxdump(0, data, len, XDUMP_ADDR_DATA_ADDR, XDUMP_DATA_LONG, tkprintf);
}

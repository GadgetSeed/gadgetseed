/** @file
    @brief	デバイスドライバAPI

    GadgetSeed は標準化されたデバイスアクセスのための API を提供します。

    GadgetSeed のデバイスドライバは struct st_device 構造体で定義されています。

    @date 	2007.03.18
    @author	Takashi SHUDO

    @page device_driver デバイスドライバ

    GadgetSeedは標準化されたデバイスドライバAPIとデバイスドライバ構造体が定義されています。

    これらは device.h で定義されています。


    ---
    @section device_driver_struct デバイスドライバ構造体

    デバイスドライバ構造体は以下の構造体で定義されています。

    @ref st_device @copybrief st_device

    GadgetSeedでアプリケーションがIOをアクセスするにはデバイスドライバを経由することを推奨します。\n
    GadgetSeedでデバイスドライバを開発することは @ref device データとそのメソッドを作成することになります。


    ---
    @section device_driver_api デバイスドライバユーザAPI

    ユーザがデバイスにアクセスするためのAPIとして、以下のAPIがあります。

    include ファイル : device.h

    | API名			| 機能				|
    |:--------------------------|:------------------------------|
    | open_device()		| @copybrief open_device	|
    | close_device()		| @copybrief close_device	|
    | lock_device()		| @copybrief lock_device	|
    | unlock_device()		| @copybrief unlock_device	|
    | read_device()		| @copybrief read_device	|
    | block_read_device()	| @copybrief block_read_device	|
    | getc_device()		| @copybrief getc_device	|
    | write_device()		| @copybrief write_device	|
    | block_write_device()	| @copybrief block_write_device	|
    | putc_device()		| @copybrief putc_device	|
    | ioctl_device()		| @copybrief ioctl_device	|
    | seek_device()		| @copybrief seek_device	|
    | sync_device()		| @copybrief sync_device	|
    | select_device()		| @copybrief select_device	|


    ---
    @section device_type デバイスタイプ

    GadgetSeedでは以下のデバイスタイプが定義されています。
    また、各デバイスタイプで実装すべき ioctl コマンドが規定されています。
    ioctl コマンドは各デバイスタイプ用の include ファイルに定義されています。

    | デバイスタイプ	| 説明					| includeファイル	|
    |:------------------|:--------------------------------------|-----------------------|
    | タイマ		| 汎用タイマデバイス			| timer_ioctl.h		|
    | UART		| シリアル通信デバイス			| uart_ioctl.h		|
    | RTC時計		| リアルタイムクロックデバイス		| rtc_ioctl.h		|
    | I2Cマスター	| I2Cマスターコントローラデバイス	| i2c_ioctl.h		|
    | SPIマスター	| SPIマスターコントローラデバイス	| spi_ioctl.h		|
    | ストレージ	| SD、MMC等のストレージデバイス		| sd_ioctl.h		|
    | ビデオ出力	| LCD等の画像表示デバイス		| video_ioctl.h		|
    | ビデオIO		| 画像表示デバイス制御用IO		| vio_ioctl.h		|
    | 圧電ブザー	| 圧電ブザー等のサウンダデバイス	| buzzer_ioctl.h	|
    | 外部割り込み(IRQ)	| 外部割り込み端子			| irq_ioctl.h		|
    | Etherデバイス	| Etherデバイス				| ether_ioctl.h		|
    | GPIO		| GPIOデバイス				| gpio_ioctl.h		|
    | AUDIO		| AUDIOデバイス				| audio_ioctl.h		|
    | タッチセンサ	| タッチセンサデバイス			| ts_ioctl.h		|
    | 環境センサ	| 温度センサ、湿度センサ、気圧センサ等	| envsnsr_ioctl.h	|
*/

#include "sysconfig.h"
#include "interrupt.h"
#include "device.h"
#include "task/syscall.h"
#include "str.h"
#include "tkprintf.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"

#ifndef GSC_KERNEL_MAX_DEVICE_NUM
#define GSC_KERNEL_MAX_DEVICE_NUM	16	///< $gsc カーネル最大デバイスドライバ数
#endif

static struct st_device *device_list[GSC_KERNEL_MAX_DEVICE_NUM];	///< 登録デバイスリスト

/**
   @brief	デバイスドライバリストを初期化する
*/
void init_device_list(void)
{
	short i;

	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		device_list[i] = 0;
	}
}

/**
   @brief	登録されているデバイス数を取得する

   @return	デバイス数
*/
int device_num(void)
{
	int i, num = 0;
	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i] != 0) {
			num ++;
		}
	}

	return num;
}

/**
   @brief	デバイス名を取得する

   @param[in]	num	デバイス名

   @return	デバイス名
*/
const char * device_name(int num)
{
	int i, n = 0;
	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i] != 0) {
			if(n == num) {
				return device_list[i]->name;
			}
			n ++;
		}
	}

	return (char *)0;
}

/**
   @brief	デバイス説明を取得する

   @param[in]	num	デバイス番号

   @return	デバイス説明
*/
const char * device_explan(int num)
{
	int i, n = 0;
	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i] != 0) {
			if(n == num) {
				return device_list[i]->explan;
			}
			n ++;
		}
	}

	return (char *)0;
}

/**
   @brief	デバイスを登録する

   @param[in]	dev	デバイスドライバ
   @param[in]	context	デバイスコンテキストデータポインタ(プライベートデータ)
   @param[in]	param	ドライバパラメタ

   @return	!=0:エラー
*/
extern int flg_init_task_run;

int register_device(const struct st_device *dev, char *param)
{
	short i;

	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i] == 0) {
			int rt = 0;
			if(dev->register_dev) {
				DKPRINTF(0x01, "Device \"%s\" install ", dev->name);
				if(dev->mutex != 0) {
					if(flg_init_task_run == 0) {
						mutex_register_ISR(dev->mutex, dev->name);
					} else {
						mutex_register(dev->mutex, dev->name);
					}
				}
				rt = dev->register_dev((struct st_device *)dev, param);
			}

			if(rt == 0) {
				device_list[i] = (struct st_device *)dev;
				DKPRINTF(0x01, "OK.\n"); 
				return 0;
			} else {
				DKPRINTF(0x01, "NG.\n"); 
				SYSERR_PRINT("\"%s\" register error.\n",
					     dev->name);
				return -1;
			}
		}
	}

	SYSERR_PRINT("Cannot register device \"%s\" (over GSC_KERNEL_MAX_DEVICE_NUM).\n",
		     dev->name);

	return -1;
}

/**
   @brief	デバイスを削除する

   @param[in]	dev	デバイスドライバ

   @return	!=0:エラー
*/
int unregister_device(const struct st_device *dev)
{
	short i;

	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i] == dev) {
			int j;
			for(j=i; j<(GSC_KERNEL_MAX_DEVICE_NUM - 1); j++) {
				device_list[j] = device_list[j+1];
			}
			device_list[GSC_KERNEL_MAX_DEVICE_NUM - 1] = 0;
			if(dev->unregister_dev) {
				int rtn = dev->unregister_dev((struct st_device *)dev);
				if(dev->mutex != 0) {
					if(flg_init_task_run == 0) {
						(void)mutex_unregister_ISR(dev->mutex);
					} else {
						mutex_unregister(dev->mutex);
					}
				}
				return rtn;
			} else {
				return 0;
			}
		}
	}

	return -1;
}

/**
   @brief 	デバイスをオープンする

   @param[in]	name	デバイス名

   @return	0:エラー,!=0:デバイスポインタ
*/
struct st_device *open_device(char *name)
{
	short i;
	int rt;

	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i] != 0) {
			if(strcomp((uchar *)name, (uchar *)device_list[i]->name) == 0) {
				if(device_list[i]->open) {
					rt = device_list[i]->open(device_list[i]);
					if(rt != 0) {
						return 0;
					}
				}
				return device_list[i];
			}
		}
	}

	return 0;
}

/**
   @brief	デバイスをクローズする

   @param[in]	dev	デバイスドライバ

   @return	!=0:エラー
*/
int close_device(struct st_device *dev)
{
	if(dev != 0) {
		if(dev->close) {
			return dev->close(dev);
		}
	}

	return -1;
}

/**
   @brief	デバイスをロックする

   @param[in]	dev	デバイスドライバ
   @param[in]	timeout	タイムアウト時間(msec)

   @return	!=0:エラー
*/
int lock_device(struct st_device *dev, unsigned int timeout)
{
	DKPRINTF(0x02, "DEV %s LOCK Timeout %ld\n", dev->name, timeout);

	if(is_in_interrupt() != 0) {
		DKPRINTF(0x02, "DEV %s LOCK abort is_in_interrupt\n", dev->name);
		return 0;
	}

	if(flg_init_task_run == 0) {
		DKPRINTF(0x02, "DEV %s LOCK abort flg_init_task_run\n", dev->name);
		return 0;
	}

	if(dev != 0) {
		if(dev->mutex != 0) {
			DKPRINTF(0x02, "DEV %s LOCK\n", dev->name);
			return mutex_lock(dev->mutex, timeout);
		} else {
			return -1;
		}
	}

	return 0;
}

/**
   @brief	デバイスをアンロックする

   @param[in]	dev	デバイスドライバ

   @return	!=0:エラー
*/
int unlock_device(struct st_device *dev)
{
	DKPRINTF(0x02, "DEV %s UNLOCK\n", dev->name);

	if(is_in_interrupt()) {
		DKPRINTF(0x02, "DEV %s UNLOCK abort is_in_interrupt\n", dev->name);
		return 0;
	}

	if(flg_init_task_run == 0) {
		DKPRINTF(0x02, "DEV %s UNLOCK abort flg_init_task_run\n", dev->name);
		return 0;
	}

	if(dev != 0) {
		if(dev->mutex != 0) {
			DKPRINTF(0x02, "DEV %s UNLOCK\n", dev->name);
			return mutex_unlock(dev->mutex);
		} else {
			return -1;
		}
	}

	return 0;
}

/**
   @brief	デバイスよりデータを読み出す

   @param[in]	dev	デバイスドライバ
   @param[out]	buf	読み出しデータポインタ
   @param[in]	count	読み出しデータバイト数

   @return	読み出しデータバイト数,<0:エラー
*/
int read_device(struct st_device *dev, void *buf, unsigned int count)
{
	if(dev != 0) {
		if(dev->read) {
			return dev->read(dev, buf, count);
		} else if(dev->getc) {
			unsigned int i;
			unsigned char *dp = buf;
			for(i=0; i<count; i++) {
				if(dev->getc(dev, dp)) {
					dp ++;
				} else {
					break;
				}
			}
			return (int)i;
		}
	}

	return -1;
}

/**
   @brief	デバイスよりブロックデータを読み出す

   @param[in]	dev		デバイスドライバ
   @param[out]	buf		読み出しデータポインタ
   @param[in]	sector		読み出しデータセクター
   @param[in]	blkcount	読み出しデータブロック数

   @return	読み出しデータバイト数,<0:エラー
*/
int block_read_device(struct st_device *dev, void *buf, unsigned int sector, unsigned int blkcount)
{
	if(dev != 0) {
		if(dev->block_read) {
			return dev->block_read(dev, buf, sector, blkcount);
		}
	}

	return -1;
}

/**
   @brief	デバイスよりデータを1バイト読み出す

   @param[in]	dev	デバイスドライバ
   @param[out]	data	読み出しデータポインタ

   @return	<0:エラー
*/
int getc_device(struct st_device *dev, unsigned char *data)
{
	if(dev != 0) {
		if(dev->getc) {
			return dev->getc(dev, data);
		} else if(dev->read) {
			return dev->read(dev, data, 1);
		}
	}

	return -1;
}

/**
   @brief	デバイスにデータを書き込む

   @param[in]	dev	デバイスドライバ
   @param[in]	buf	書き込みデータポインタ
   @param[in]	count	書き込みデータバイト数

   @return	書き込みデータバイト数,<0:エラー
*/
int write_device(struct st_device *dev, const void *buf, unsigned int count)
{
	if(dev != 0) {
		if(dev->write) {
			return dev->write(dev, buf, count);
		} else if(dev->putc) {
			unsigned int i;
			const unsigned char *dp = buf;
			for(i=0; i<count; i++) {
				if(dev->putc(dev, *dp)) {
					dp ++;
				} else {
					break;
				}
			}
			return (int)i;
		}
	}

	return -1;
}

/**
   @brief	デバイスにブロックデータを書き込む

   @param[in]	dev		デバイスドライバ
   @param[in]	buf		書き込みデータポインタ
   @param[in]	sector		書き込みデータセクター
   @param[in]	blkcount	書き込みデータブロック数

   @return	書き込みデータバイト数,<0:エラー
*/
int block_write_device(struct st_device *dev, const void *buf, unsigned int sector, unsigned int blkcount)
{
	if(dev != 0) {
		if(dev->block_write) {
			return dev->block_write(dev, buf, sector, blkcount);
		}
	}

	return -1;
}

/**
   @brief	デバイスにデータを1バイト書き込む

   @param[in]	dev	デバイスドライバ
   @param[out]	data	読み出しデータ

   @return	<0:エラー
*/
int putc_device(struct st_device *dev, unsigned char data)
{
	if(dev != 0) {
		if(dev->putc) {
			return dev->putc(dev, data);
		} else if(dev->write) {
			unsigned char buf = data;
			return dev->write(dev, &buf, 1);
		}
	}

	return -1;
}

/**
   @brief	デバイスを制御する

   @param[in]	dev	デバイスドライバ
   @param[in]	com	コマンド
   @param[in]	arg	コマンド引数

   @return	<0:エラー
*/
int ioctl_device(struct st_device *dev, unsigned int com, unsigned int arg, void *param)
{
	if(dev != 0) {
		if(dev->ioctl) {
			return dev->ioctl(dev, com, arg, param);
		}
	}

	return -1;
}

/**
   @brief	デバイスのアクセスポイントを設定する

   @param[in]	dev	デバイスドライバ
   @param[in]	offset	オフセット
   @param[in]	whence	オフセット開始位置

   @return	<0:エラー
*/
int seek_device(struct st_device *dev, int offset, int whence)
{
	if(dev != 0) {
		if(dev->seek) {
			return dev->seek(dev, offset, whence);
		}
	}

	return -1;
}

/**
   @brief	デバイスの書き込みデータの同期をとる

   @param[in]	dev	デバイスドライバ

   @return	<0:エラー
*/
int sync_device(struct st_device *dev)
{
	if(dev != 0) {
		if(dev->sync) {
			return dev->sync(dev);
		}
	}

	return -1;
}

/**
   @brief	デバイスのアクセス準備完了を待つ

   @param[in]	dev	デバイスドライバ
   @param[in]	timeout	タイムアウト時間

   @return	<0:エラー
*/
int select_device(struct st_device *dev, unsigned int timeout)
{
	if(dev != 0) {
		if(dev->select) {
			return dev->select(dev, timeout);
		}
	}

	return -1;
}

/**
   @brief	デバイスを休止状態にする

   @param[in]	dev	デバイスドライバ

   @return	<0:エラー
*/
int suspend_device(struct st_device *dev)
{
	if(dev != 0) {
		if(dev->suspend) {
			return dev->suspend(dev);
		}
	}

	return -1;
}

/**
   @brief	デバイスを活性化する

   @param[in]	dev	デバイスドライバ

   @return	<0:エラー
*/
int resume_device(struct st_device *dev)
{
	if(dev != 0) {
		if(dev->resume) {
			return dev->resume(dev);
		}
	}

	return -1;
}

/**
   @brief	全デバイスを休止状態にする

   @return	<0:エラー
*/
int suspend(void)
{
	int i;

	for(i=(GSC_KERNEL_MAX_DEVICE_NUM-1); i>=0; i--) {
		if(device_list[i]) {
			DKPRINTF(0x01, "suspend : %s\n", device_list[i]->name);
			(void)suspend_device(device_list[i]);
		}
	}

	return 0;
}

#include "datetime.h"

/**
   @brief	全デバイスを活性化する

   @return	<0:エラー
*/
int resume(void)
{
	int i;

	for(i=0; i<GSC_KERNEL_MAX_DEVICE_NUM; i++) {
		if(device_list[i]) {
			DKPRINTF(0x01, "resume : %s\n", device_list[i]->name);
			(void)resume_device(device_list[i]);
		}
	}

#ifdef GSC_DEV_ENABLE_RTC
	sync_systime_from_rtc();
#endif

	return 0;
}

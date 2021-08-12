/** @file
    @brief デバイスドライバAPI

    @date 	2007.03.18
    @author	Takashi SHUDO
*/

#ifndef DEVICE_H
#define DEVICE_H

#include "task/mutex.h"

// 推奨デバイス名
#define DEF_DEV_NAME_LED	"led"		// LED等
#define DEF_DEV_NAME_EEPRMOM	"eeprom"	// EEPROM

#define MAX_DEVNAMELRN	10

// seek() whence の値
#define SEEK_SET	0	///< 設定
#define SEEK_CUR	1	///< 現在値に加算
#define SEEK_END	2	///< ファイルサイズに加算

struct st_device {
	char name[MAX_DEVNAMELRN];	///< デバイス名文字列
	char explan[32];	///< デバイス説明文字列
	void *info;		///< デバイス情報データポインタ
	void *private_data;	///< ドライバ固有データポインタ
	struct st_mutex *mutex;	///< デバイス排他アクセス用MUTEX
	int (* register_dev)(struct st_device *dev, char *param);
	int (* unregister_dev)(struct st_device *dev);
	int (* lock)(struct st_device *dev, unsigned int timeout);
	int (* unlock)(struct st_device *dev);
	int (* open)(struct st_device *dev);
	int (* close)(struct st_device *dev);
	int (* read)(struct st_device *dev, void *buf, unsigned int count);
	int (* getc)(struct st_device *dev, unsigned char *buf);
	int (* write)(struct st_device *dev, const void *buf, unsigned int count);
	int (* putc)(struct st_device *dev, unsigned char data);
	int (* ioctl)(struct st_device *dev, unsigned int com, unsigned int arg, void *param);
	int (* seek)(struct st_device *dev, int offset, int whence);
	int (* block_read)(struct st_device *dev, void *buf, unsigned int sector, unsigned int blkcount);
	int (* block_write)(struct st_device *dev, const void *buf, unsigned int sector, unsigned int blkcount);
	int (* epbuf_get)(struct st_device *dev, void **buf);
	int (* epbuf_release)(struct st_device *dev, void *buf);
	int (* sync)(struct st_device *dev);
	int (* select)(struct st_device *dev, unsigned int timeout);
	int (* suspend)(struct st_device *dev);
	int (* resume)(struct st_device *dev);
}; ///< デバイスドライバ構造体

void init_device_list(void);
extern int device_num(void);
extern const char * device_name(int num);
extern const char * device_explan(int num);
extern int register_device(const struct st_device *dev, char *param);
extern int unregister_device(const struct st_device *dev);
extern int lock_device(struct st_device *dev, unsigned int timeout);
extern int unlock_device(struct st_device *dev);
extern struct st_device * open_device(char *name);
extern int close_device(struct st_device *dev);
extern int read_device(struct st_device *dev, void *buf, unsigned int count);
extern int getc_device(struct st_device *dev, unsigned char *data);
extern int write_device(struct st_device *dev, const void *buf, unsigned int count);
extern int putc_device(struct st_device *dev, unsigned char data);
extern int ioctl_device(struct st_device *dev, unsigned int com, unsigned int arg, void *param);
extern int seek_device(struct st_device *dev, int offset, int whence);
extern int block_read_device(struct st_device *dev, void *buf, unsigned int sector, unsigned int blkcount);
extern int block_write_device(struct st_device *dev, const void *buf, unsigned int sector, unsigned int blkcount);
extern int epbuf_get(struct st_device *dev, void **buf);
extern int epbuf_release(struct st_device *dev, void *buf);
extern int sync_device(struct st_device *dev);
extern int select_device(struct st_device *dev, unsigned int timeout);
extern int suspend_device(struct st_device *dev);
extern int resume_device(struct st_device *dev);

extern int suspend(void);
extern int resume(void);

#endif // DEVICE_H

/** @file
    @brief	ファイル

    @date	2008.03.20
    @author	Takashi SHUDO
*/

#ifndef FILE_H
#define FILE_H

#include "device.h"
#include "str.h"
#include "ff.h"

extern void init_file(void);
extern int open_file(const uchar *path, int flags);
extern int read_file(int fd, void *buf, unsigned int count);
extern int write_file(int fd, const void *buf, unsigned int count);
extern int seek_file(int fd, unsigned int offset, int whence);
extern int tell_file(int fd);
extern int close_file(int fd);
extern DIR * opendir_file(const unsigned char *name);
extern int readdir_file(DIR *dir, FILINFO *info);
extern int stat_file(const unsigned char *path, FILINFO *info);
extern int getfree_file(const unsigned char *path, unsigned long *sect, FATFS **fs);
extern int sync_file(int fd);
extern int unlink_file(const uchar *path);
extern int mkdir_file(const uchar *path);
extern int chmod_file(const uchar *path, unsigned char flag);
extern int rename_file(const uchar *oldpath, const uchar *newpath);
extern int mkfs_file(const uchar *path, unsigned char part, unsigned short alloc);

extern uchar *get_last_filename(uchar *filename, const uchar *fullpath, unsigned int len);
extern uchar *get_filename_extension(uchar *ext, const uchar *filename, unsigned int len);

#endif // FILE_H

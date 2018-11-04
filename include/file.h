/** @file
    @brief	ファイル

    @date	2008.03.20
    @author	Takashi SHUDO
*/

#ifndef FILE_H
#define FILE_H

#include "fs.h"
#include "str.h"

extern void init_file(void);
extern struct st_filesystem * get_filesystem(const uchar *path);
extern int open_file(const uchar *path, int flags);
extern t_ssize read_file(int fd, void *buf, t_size count);
extern t_ssize write_file(int fd, const void *buf, t_size count);
extern t_ssize seek_file(int fd, t_ssize offset, int whence);
extern t_size tell_file(int fd);
extern t_ssize size_file(int fd);
extern int close_file(int fd);
extern FS_DIR * opendir_file(const uchar *name);
extern int readdir_file(FS_DIR *dir, FS_FILEINFO *finfo);
extern int closedir_file(FS_DIR *dir);
extern int stat_file(const uchar *path, FS_FILEINFO *finfo);
extern int getfree_file(const uchar *path, unsigned long *sect, void **fs);
extern int sync_file(int fd);
extern int unlink_file(const uchar *path);
extern int mkdir_file(const uchar *path);
extern int chmod_file(const uchar *path, unsigned char flag);
extern int rename_file(const uchar *oldpath, const uchar *newpath);
extern int mkfs_file(const uchar *path, unsigned char part, unsigned short alloc);

extern uchar *get_last_filename(uchar *filename, const uchar *fullpath, unsigned int len);
extern uchar *get_filename_extension(uchar *ext, const uchar *filename, unsigned int len);

#define SIZE_STR	"X.XM"
#define SIZE_STR_LEN	((unsigned int)sizeof(SIZE_STR))
char *size2str(char *str, t_size size);

#endif // FILE_H

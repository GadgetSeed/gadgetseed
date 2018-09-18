/** @file
    @brief	ファイルリスト

    @date	2017.11.18
    @author	Takashi SHUDO
*/

#include "filelist_data.h"
#include "memory.h"
#include "str.h"
#include "tprintf.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


static void print_finfo(FS_FILEINFO *fi)
{
	DTPRINTF(0x01, "%c", (fi->fattrib & AM_DIR) ? 'D' : '-');
	DTPRINTF(0x01, "%c", (fi->fattrib & AM_RDO) ? 'R' : '-');
	DTPRINTF(0x01, "%c", (fi->fattrib & AM_HID) ? 'H' : '-');
	DTPRINTF(0x01, "%c", (fi->fattrib & AM_SYS) ? 'S' : '-');
	DTPRINTF(0x01, "%c", (fi->fattrib & AM_ARC) ? 'A' : '-');

	DTPRINTF(0x01, " %04d/%02d/%02d",
		(int)(fi->fdate >> 9) + 1980,
		(int)(fi->fdate >> 5) & 15,
		(int)fi->fdate & 31);

	DTPRINTF(0x01, " %02d:%02d:%02d",
		(int)(fi->ftime >> 11),
		(int)(fi->ftime >> 5) & 63,
		(int)fi->ftime & 63);

	DTPRINTF(0x01, " %s\n", sj2utf8((uchar *)fi->fname));
}

int create_filelist_data(struct st_fileinfo *fileinfo, const unsigned char *path, int (* filter)(FS_FILEINFO *finfo))
{
	FS_DIR *dir;
	int res;
	int filecount = 0;
	struct st_fileinfo *tmp_filist, *filist = fileinfo;

	dir = opendir_file(path);
	if(dir == 0) {
		DTPRINTF(0x01, "Cannot open \"%s\".\n", path);
		return -1;
	}

	for(;;) {
		tmp_filist = (struct st_fileinfo *)
				alloc_memory(sizeof(struct st_fileinfo));
		DTPRINTF(0x01, "ALLOC %d %p\n", filecount, tmp_filist);

		if(tmp_filist != 0) {
			//DTPRINTF(0x01, "%p\n", tmp_filist);

			res = readdir_file(dir, &(tmp_filist->file_info));

			if((res != FR_OK) || !tmp_filist->file_info.fname[0]) {
				DTPRINTF(0x01, "FREE %p\n", tmp_filist);
				free_memory(tmp_filist);
				break;
			} else {
				int filter_ng = 0;
				print_finfo(&tmp_filist->file_info);

				if(filter != 0) {
					filter_ng = filter(&tmp_filist->file_info);
				}

				if(filter_ng != 0) {
					free_memory(tmp_filist);
					continue;
				} else {
					filecount ++;
					filist->next = tmp_filist;
					filist = tmp_filist;
					filist->next = 0;
				}
			}
		} else {
			closedir_file(dir);
			return -1;
			break;
		}
	}

	closedir_file(dir);

	return filecount;
}

void dispose_filelist_data(struct st_fileinfo *fileinfo)
{
	struct st_fileinfo *filist, *tmp_filist;

	filist = fileinfo->next;

	while(filist != 0) {
		tmp_filist = filist->next;
		DTPRINTF(0x01, "FREE %p\n", filist);
		free_memory(filist);
		filist = tmp_filist;
	}
}

/** @file
    @brief	ファイルシステムAPI

    @date 	2018.09.09
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "fs.h"
#include "str.h"

#ifdef GSC_COMP_ENABLE_FATFS
extern const struct st_filesystem fatfs_fs;
#endif

#ifdef GSC_COMP_ENABLE_PIPEFS
extern const struct st_filesystem pipefs_fs;
#endif

struct st_filesystem const *filesystems[] = {
#ifdef GSC_COMP_ENABLE_FATFS
	&fatfs_fs,
#endif
#ifdef GSC_COMP_ENABLE_PIPEFS
	&pipefs_fs,
#endif
	0
};

struct st_filesystem * search_filesystem(const char *name)
{
	const struct st_filesystem **fs = &filesystems[0];

	while(*fs != 0) {
		if(strcomp((uchar *)name, (uchar *)(*fs)->name) == 0) {
			return (struct st_filesystem *)*fs;
		}
		fs ++;
	}

	return 0;
}

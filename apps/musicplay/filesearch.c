/** @file
    @brief	ファイル検索

    @date	2012.11.11
    @auther	Takashi SHUDO
*/

#include "filesearch.h"
#include "str.h"
#include "file.h"
#include "tprintf.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"


int file_kind(unsigned char *fname, struct file_ext *ext)
{
	struct file_ext *fep = ext;
	struct file_ext fext, cext;

	while(fep->ext[0] != 0) {
		unsigned char *p;
		if(strleng(fep->ext) > FNAME_EXT_LEN) {
			eprintf("file extention \"%s\" length too long\n",
				fep->ext);
		} else if(strleng(fname) > strleng(fep->ext)) {
			int extlen = strleng(fep->ext);
			p = fname + strleng(fname) - extlen;;
			strncopy(fext.ext, p, extlen);
			str2cap(fext.ext);
			strncopy(cext.ext, fep->ext, extlen);
			str2cap(cext.ext);
			DTPRINTF(0x01, "(%s)", fext.ext);
			DTPRINTF(0x01, "{%s}", cext.ext);
			if(strcomp(fext.ext, cext.ext) == 0) {
				DTPRINTF(0x01, "{%s}", p);
				return 1;
			}
		}
		fep ++;
	}

	return 0;
}

static int pathjoin(unsigned char *dst, int len, unsigned char *src1, unsigned char *src2)
{
	int i, j = 0;

	if((strleng(src1) + strleng(src2)) > len) {
		return -1;
	}

	for(i=0; i<(len-1); i++) {
		if(src1[i] != 0) {
			dst[i] = src1[i];
		} else {
			break;
		}
	}

	if(i<(len-1)) {
		dst[i] = '/';
	}

	i++;

	for(; i<(len-1); i++) {
		if(src2[j] != 0) {
			dst[i] = src2[j];
			j ++;
		} else {
			break;
		}
	}

	dst[i] = 0;

	return i;
}

static int dir_depth;

static int search_dir(unsigned char *path, struct file_ext *ext, fileadd_func func)
{
	DIR dir;
	int res;

#ifdef PRINT_MUSICFILE_INFO
	tprintf("[%s]\n", sj2utf8(path));
#endif
	res = f_opendir(&dir, (char *)path);
	if(res != FR_OK) {
		eprintf("Cannot open \"%s\".\n", path);
		return -1;
	}

	for(;;) {
		FILINFO file_info;
		unsigned char tmp[MAX_PATHNAME_LEN];
		int len;

		res = readdir_file(&dir, &file_info);
		if(res != FR_OK) {
			eprintf("Cannot read \"%s\".\n", path);
			return -1;
		}

		if(file_info.fname[0] == 0) {
			break;
		}

		len = pathjoin(tmp, MAX_PATHNAME_LEN, path, (unsigned char *)file_info.fname);
		if(len < 0) {
			eprintf("File path too long \"%s/%s\".\n", path,  (unsigned char *)file_info.fname);
		}

		if(file_info.fattrib & AM_DIR) {
#ifdef PRINT_MUSICFILE_INFO
			tprintf("DIR %s\n", sj2utf8((unsigned char *)file_info.fname));
#endif
			dir_depth ++;
			DTPRINTF(0x01, "DIR %s len=%d depth=%d\n", tmp, len,
				 dir_depth);
			(void)len;
			search_dir(tmp, ext, func);
			dir_depth --;
		} else {
			if(file_kind((unsigned char *)file_info.fname, ext)) {
				func(tmp);
			}
		}
	}

	return 0;
}

int search_file(unsigned char *path, struct file_ext *ext, fileadd_func func)
{
	dir_depth = 0;
	
	search_dir(path, ext, func);

	return 0;
}

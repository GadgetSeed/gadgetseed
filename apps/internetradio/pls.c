/** @file
    @brief	PLSファイルデコード

    @date	2019.01.03
    @auther	Takashi SHUDO
*/

#include "file.h"
#include "memory.h"
#include "pls.h"
#include "tprintf.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


int pls_decode(struct st_music_info *info, int fd)
{
	int rt = 0;
	t_ssize size;
	unsigned char *buf;
	uchar str_playlist[] = "[playlist]";
	uchar str_file[] = "File1=";
	uchar str_title[] = "Title1=";
	int rtn = 0;

	size = size_file(fd);
	DTPRINTF(0x01, "SIZE = %lld\n", size);

	buf = alloc_memory(size+1);
	if(buf == 0) {
		DTPRINTF(0x01, "buf= %p\n", buf);
		return -1;
	}

	rt = read_file(fd, buf, size);
	if(rt != 0) {
		int srt = 0;
		buf[rt] = 0;
		DTPRINTF(0x01, "%s\n", buf);
		srt = strncomp(buf, str_playlist, strleng(str_playlist));
		if(srt == 0) {
			uchar *up = 0;
			uchar *ep = 0;
			up = searchstr(buf, str_file);
			if(up != 0) {
				up += strleng(str_file);
				ep = strchar(up, (uchar)'\n');
				if(ep != 0) {
					*ep = 0;
				}
				DTPRINTF(0x01, "URL : \"%s\"\n", up);
				strncopy(info->url, (const uchar *)up, MAX_MINFO_STR);
			}
			up = searchstr(ep+1, str_title);
			if(up != 0) {
				up += strleng(str_title);
				ep = strchar(up, (uchar)'\n');
				if(ep != 0) {
					*ep = 0;
				}
				if(*up == '(') {
					uchar *p;
					p = searchstr(up, (uchar *)") ");
					if(p != 0) {
						up = (p+2);
					}
				}
				DTPRINTF(0x01, "ALBUM : \"%s\"\n", up);
				strncopy(info->album, (const uchar *)up, MAX_MINFO_STR);
				rtn = 1;
			}
		}
	}
	free_memory(buf);

	return rtn;
}

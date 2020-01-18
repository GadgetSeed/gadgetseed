/** @file
    @brief	M3Uファイルデコード

    @date	2018.12.30
    @auther	Takashi SHUDO
*/

#include "file.h"
#include "memory.h"
#include "m3u.h"
#include "tprintf.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


int m3u_decode(struct st_music_info *info, int fd)
{
	int rt = 0;
	t_ssize size;
	unsigned char *buf;
	uchar str_extinf[] = "#EXTINF:";
	uchar str_http[] = "http://";
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
		uchar *p = 0;
		uchar *ep = 0;
		buf[rt] = 0;
		DTPRINTF(0x01, "%s\n", buf);
		p = searchstr(buf, str_extinf);
		if(p != 0) {
			uchar *up;
			up = strchar(p, (uchar)',');
			if(up != 0) {
				up ++;
				if(*up == '(') {
					p = searchstr(up, (uchar *)") ");
					if(p != 0) {
						up = (p+2);
					}
				}
				ep = strchar(up, (uchar)'\n');
				if(ep != 0) {
					*ep = 0;
				}
				DTPRINTF(0x01, "STATION : \"%s\"\n", up);
				strncopy(info->album, (const uchar *)up, MAX_MINFO_STR);
			}
		}
		if(ep != 0) {
			p = searchstr(ep+1, str_http);
			if(p != 0) {
				uchar *ep;
				ep = strchar(p, (uchar)'\n');
				if(ep != 0) {
					*ep = 0;
				}
				DTPRINTF(0x01, "HTTP : \"%s\"\n", p);
				strncopy(info->url, (const uchar *)p, MAX_MINFO_STR);
				rtn = 1;
			}
		}
	}
	free_memory(buf);

	return rtn;
}

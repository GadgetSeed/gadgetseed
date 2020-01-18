/** @file
    @brief	ShoutCast プロトコル

    @date	2018.09.02
    @auther	Takashi SHUDO
*/

#include "fifo.h"
#include "tprintf.h"
#include "sysevent.h"
#include "log.h"
#include "task/syscall.h"
#include "lwip/netdb.h"

#include "shoutcast.h"
#include "ir_socket.h"

#include "music_info.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

#define MAX_BUF	(256*1)

int stream_read_size = 0;

int start_shoutcast_stream(uchar *path)
{
	int ws = 0;
	char send_buf[MAX_BUF + 1];

	DTFPRINTF(0x02, "path = %s\n", path);

	stream_read_size = 0;

	tsnprintf(send_buf, MAX_BUF, "GET /%s HTTP/1.0\r\n", path);
	DTPRINTF(0x04, "%s", send_buf);
	ws = write_intrnet_radio(send_buf, strleng((const uchar *)send_buf));
	if(ws < 0) {
		gslog(1, "send error\n");
		return -1;
	}

	tsnprintf(send_buf, MAX_BUF, "Host: localhost\r\n");
	DTPRINTF(0x04, "%s", send_buf);
	ws = write_intrnet_radio(send_buf, strleng((const uchar *)send_buf));
	if(ws < 0) {
		gslog(1, "send error\n");
		return -1;
	}

	tsnprintf(send_buf, MAX_BUF, "User-Agent: GadgetSeed\r\n");
	DTPRINTF(0x04, "%s", send_buf);
	ws = write_intrnet_radio(send_buf, strleng((const uchar *)send_buf));
	if(ws < 0) {
		gslog(1, "send error\n");
		return -1;
	}

	tsnprintf(send_buf, MAX_BUF, "Accept: */*\r\n");
	DTPRINTF(0x04, "%s", send_buf);
	ws = write_intrnet_radio(send_buf, strleng((const uchar *)send_buf));
	if(ws < 0) {
		gslog(1, "send error\n");
		return -1;
	}

	tsnprintf((char *)send_buf, MAX_BUF, "Icy-MetaData: 1\r\n");
	DTPRINTF(0x04, "%s", send_buf);
	ws = write_intrnet_radio(send_buf, strleng((const uchar *)send_buf));
	if(ws < 0) {
		gslog(1, "send error\n");
		return -1;
	}

	tsnprintf((char *)send_buf, MAX_BUF, "\r\n");
	DTPRINTF(0x04, "%s", send_buf);
	ws = write_intrnet_radio(send_buf, strleng((const uchar *)send_buf));
	if(ws < 0) {
		gslog(1, "send error\n");
		return -1;
	}

	return 0;
}


static int read_line(char *buf)
{
	int len = 0;

	while(1) {
		char tmp;
		int rs;

		rs = read_intrnet_radio(&tmp, 1);
		if(rs > 0) {
			buf[len] = tmp;
			len ++;
			if(tmp == '\n') {
				buf[len] = 0;
				break;
			}
			if(len >= (MAX_BUF-1)) {
				buf[len+1] = 0;
				break;
			}
		} else if(rs < 0) {
			return -1;
		} else {
			break;
		}
	}

	return len;
}

static char buf[MAX_BUF + 1];

int decode_shoutcast_message(struct st_music_info *music_info)
{
	int len = 0;

	init_music_info(music_info);

	while(1) {
		uchar *sp, *ep;
		len = read_line(buf);
		if(len < 0) {
			return 1;
		}
		gslog(1, "# %s", buf);
		if(strncomp((uchar *)"icy-name:", (uchar *)buf, 9) == 0) {
			sp = (uchar *)&buf[9];
			if(*sp == ' ') {
				sp++;
			}
			strncopy(music_info->album, sp, MAX_BUF);
			ep = strchar(music_info->album, (uchar)'\r');
			*ep = 0;
		} else if(strncomp((uchar *)"icy-genre:", (uchar *)buf, 10) == 0) {
			sp = (uchar *)&buf[10];
			if(*sp == ' ') {
				sp++;
			}
			strncopy(music_info->genre, sp, MAX_BUF);
			ep = strchar(music_info->genre, (uchar)'\r');
			*ep = 0;
		} else if(strncomp((uchar *)"icy-br:", (uchar *)buf, 7) == 0) {
			ep = strchar((uchar *)&buf[7], (uchar)'\r');
			*ep = 0;
			music_info->bit_rate = dstoi((uchar *)&buf[7]);
		} else if(strncomp((uchar *)"icy-sr:", (uchar *)buf, 7) == 0) {
			ep = strchar((uchar *)&buf[7], (uchar)'\r');
			*ep = 0;
			music_info->sampling_rate = dstoi((uchar *)&buf[7]);
		} else if(strncomp((uchar *)"icy-vbr:", (uchar *)buf, 8) == 0) {
			ep = strchar((uchar *)&buf[8], (uchar)'\r');
			*ep = 0;
			music_info->vbr = dstoi((uchar *)&buf[8]);
			if(music_info->vbr != 0) {
				eprintf("VBR is not supported\n");
				return 1;
			}
		} else if(strncomp((uchar *)"icy-url:", (uchar *)buf, 8) == 0) {
			sp = (uchar *)&buf[8];
			if(*sp == ' ') {
				sp++;
			}
			strncopy(music_info->url, sp, MAX_BUF);
			ep = strchar(music_info->url, (uchar)'\r');
			*ep = 0;
		} else if(strncomp((uchar *)"icy-metaint:", (uchar *)buf, 12) == 0) {
			ep = strchar((uchar *)&buf[12], (uchar)'\r');
			*ep = 0;
			music_info->metaint = dstoi((uchar *)&buf[12]);
		} else if((strncomp((uchar *)"content-type:", (uchar *)buf, 13) == 0) ||
			  (strncomp((uchar *)"Content-Type:", (uchar *)buf, 13) == 0)) {
			ep = strchar((uchar *)&buf[13], (uchar)'\r');
			*ep = 0;
			sp = (uchar *)&buf[13];
			if(*sp == ' ') {
				sp++;
			}
			if(strcomp((uchar *)"audio/mpeg", sp) == 0) {
				music_info->format = MUSIC_FMT_MP3;
				music_info->frame_size = 1152;
			} else {
				eprintf("No support format \"%s\"\n", sp);
				return 1;
			}
		}

		if(len == 2) {
			DTFPRINTF(0x02, "len = %d\n", len);
			XDUMP(0x02, (uchar *)buf, 2);
			return 1;
		}
	}

	return 0;
}


void decode_metadata(struct st_music_info *music_info, char *metadata, int len)
{
	//gslog(8, "%d : %s\n", len, metadata);

	uchar sstr[] = "StreamTitle='";
	uchar *p;
	p = searchstr((uchar *)metadata, sstr);
	if(p != 0) {
		static uchar tmp[MAX_MINFO_STR];
		uchar *tp;
		strncopy(tmp, p+strleng(sstr), MAX_MINFO_STR-1);
		tp = searchstr(tmp, (uchar *)" - ");
		if(tp != 0) {
			*tp = 0;
			tprintf("\nArtist : %s\n", tmp);
			strncopy(music_info->artist, tmp, MAX_MINFO_STR-1);
			p = strchar(tp+3, (uchar)';');
			if(p != 0) {
				*p = 0;
				if(*(p-1) == '\'') {
					*(p-1) = 0;
				}
				if(tp != 0) {
					tprintf("Title : %s\n", tp+3);
					strncopy(music_info->title, tp+3, MAX_MINFO_STR-1);
				}
			} else {
				tprintf("Title : %s\n", tp+3);
			}
		}
	}
}

static char metadata[MAX_BUF + 1];

int decode_shoutcast_stream(struct st_music_info *music_info, unsigned char *stream)
{
	int i;
	int rs;

	DTPRINTF(0x01, "frame_size = %d\n", music_info->frame_size);

	for(i=0; i<music_info->frame_size; i++) {
		rs = read_intrnet_radio((char *)stream, 1);
		//tprintf("%d", rs);

		if(rs > 0) {
			stream_read_size += rs;
			stream += rs;
			//gslog(1, "Stream len = %d\n", *len);

			if((stream_read_size % music_info->metaint) == 0) {
				char mtlen;
				rs = read_intrnet_radio(&mtlen, 1);
				if(rs < 1) {
					return i;
				}
				if(mtlen != 0) {
					gslog(0, "mtlen %d\n", mtlen);
					if((mtlen * 16) <= MAX_BUF) {
						rs = read_intrnet_radio(metadata, mtlen * 16);
						if(rs == (mtlen * 16)) {
							decode_metadata(music_info, metadata, rs);
							create_event(EVT_SOUND_PREPARED, 0, (void *)music_info);
						}
					} else {
						gslog(0, "metlen too learge %d\n", mtlen);
					}
				} else {
					//gslog(8, "metadata %d\n", mtlen);
				}
			}
		} else if(rs < 0) {
			return rs;
		} else {
			return i;
		}
	}

	return i;
}

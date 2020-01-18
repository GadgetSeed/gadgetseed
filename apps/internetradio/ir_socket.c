/** @file
    @brief	Internet radio socket

    @date	2018.10.27
    @auther	Takashi SHUDO
*/

#define GSLOG_PREFIX	"ISO: "
#include "ir_socket.h"

#include "fifo.h"
#include "tprintf.h"
#include "tkprintf.h"
#include "log.h"

#include "lwip/netdb.h"

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

#define MAX_STREAM_BUF	(1024*4)

static int ir_socket = -1;
static unsigned char ir_buf[MAX_STREAM_BUF + 1];
static struct st_fifo ir_fifo;

void perse_internet_radio_url(const uchar *url_path, uchar *url, uchar *port, uchar *path)
{
	uchar *p;
	uchar *argp;

	argp = (uchar *)url_path;
	if(strncomp((uchar *)"http://", argp, 7) == 0) {
		argp += 7;
	}
	strncopy(url, argp, MAX_URL_STR_LEN);
	p = strchar(url, (uchar)':');
	if(p != 0) {
		*p = 0;
		strncopy(port, p+1, MAX_URL_STR_LEN);
		p = strchar(port, (uchar)'/');
		if(p != 0) {
			*p = 0;
			strncopy(path, p+1, MAX_URL_STR_LEN);
		}
	} else {
		strncopy(port, (uchar *)"80", 3);
		p = strchar(url, (uchar)'/');
		if(p != 0) {
			*p = 0;
			strncopy(path, p+1, MAX_URL_STR_LEN);
		}
	}

	DTPRINTF(0x01, "URL  : %s\n", url);
	DTPRINTF(0x01, "PORT : %s\n", port);
	DTPRINTF(0x01, "PATH : %s\n", path);
}

int open_internet_radio(const uchar *url, const uchar *port)
{
	int rtn = 0;
	struct addrinfo *ainfo = 0;

	DTFPRINTF(0x02, "url = %s, port = %s\n", url, port);

	init_fifo(&ir_fifo, ir_buf, MAX_STREAM_BUF + 1);

	GSLOG(0, "URL  : %s\n", url);
	GSLOG(0, "PORT : %s\n", port);

	rtn = lwip_getaddrinfo((const char *)url, (const char *)port, 0, &ainfo);

	if(rtn == 0) {
		struct sockaddr_in *addr = (struct sockaddr_in *)ainfo->ai_addr;
		//GSLOG(0, "%s %d\n", ainfo->ai_canonname, rtn);
		GSLOG(0, "Connect to %s\n", inet_ntoa(addr->sin_addr));
		//GSLOG(0, "%d %d %d\n", ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol);
	} else {
		eprintf("lwip_getaddrinfo error %d\n", rtn);
		return -1;
	}

	ir_socket = lwip_socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(ir_socket < 0) {
		eprintf("lwip_socket error %d\n", ir_socket);
		lwip_freeaddrinfo(ainfo);
		return -1;
	} else {
		GSLOG(0, "lwip_socket(%d) success\n", ir_socket);
	}

	if(0) {
		struct timeval to;
		to.tv_sec = 5;
		to.tv_usec = 0;

		rtn = lwip_setsockopt(ir_socket, SOL_SOCKET, SO_RCVTIMEO, &to, sizeof(to));
		if(rtn != 0) {
			eprintf("lwip_setsockopt error %d\n", rtn);
			lwip_freeaddrinfo(ainfo);
			return -1;
		}
	}

	rtn = lwip_connect(ir_socket, ainfo->ai_addr, ainfo->ai_addrlen);
	lwip_freeaddrinfo(ainfo);
	if(rtn != 0) {
		eprintf("lwip_connect error %d\n", rtn);
		return -1;
	}

	return ir_socket;
}

void close_internet_radio(void)
{
	int rt = 0;

	DTFPRINTF(0x02, "\n");

	GSLOG(0, "lwip_close(%d) start\n", ir_socket);
	rt = lwip_close(ir_socket);
	if(rt != 0) {
		eprintf("lwip_close error(%d)\n", rt);
	} else {
		GSLOG(0, "lwip_close(%d) success\n", ir_socket);
	}
				
	ir_socket = -1;
}

int write_intrnet_radio(char *data, unsigned int len)
{
	int ws;

	DTFPRINTF(0x02, "data=%p len=%d\n", data, len);

	ws = lwip_write(ir_socket, data, len);

	return ws;
}

#include "timer.h"

static unsigned char tmp_buf[MAX_STREAM_BUF];

int read_intrnet_radio(char *data, unsigned int len)
{
	int fsize;
	int rs, rt;

	fsize = fifo_size(&ir_fifo);
	DTPRINTF(0x04, "fifo_free = %d\n", fsize);
	if(fsize < (MAX_STREAM_BUF/2)) {
		int rsize = 1414;
		//unsigned long long stime, etime, xtime;
		DTPRINTF(0x04, "lwip_read %d", rsize);
		//tkprintf("%4d : ", rsize);
		//stime = get_system_utime();
		rs = lwip_read(ir_socket, tmp_buf, rsize);
		//etime = get_system_utime();
		//xtime = etime - stime;
		//GSLOG(8, "%4d : %4d.%03d\n", rs, (int)xtime/1000, (int)xtime%1000);
		DTPRINTF(0x04, " -> %d\n", rs);
		if(rs > 0) {
			DTPRINTF(0x04, " rs = %d\n", rs);
			write_fifo(&ir_fifo, tmp_buf, rs);
		} else {
			eprintf("lwip_read error %d\n", rs);
			return -1;
		}
	}

	rt = read_fifo(&ir_fifo, (unsigned char *)data, len);

	return rt;
}

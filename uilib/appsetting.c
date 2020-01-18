/** @file
    @brief	アプリケーション設定保存、復旧

    @date	2019.01.19
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "appsetting.h"
#include "tprintf.h"
#include "str.h"
#include "file.h"
#include "log.h"

#ifdef GSC_COMP_ENABLE_TCPIP
#include "lwip/ip.h"
#endif

//#define DEBUGTBITS 0x02
#include "dtprintf.h"

#define MAXCONFBUF	63

static char confbuf[MAXCONFBUF+1];

/*
  BYTE
*/
int byte_save(void *val, uchar *str)
{
	int rtn;

	DTPRINTF(0x02, "%s : %d\n", CFGTYPE_BYTE, *(char *)val);

	rtn = tsnprintf((char *)str, MAXCONFBUF, "%d", (int)(*(char *)val));

	return rtn;
}

int byte_load(void *val, uchar *str)
{
	DTPRINTF(0x02, "%s : %s\n", CFGTYPE_BYTE, (char *)str);

	*(char *)val = (char)dstoi(str);

	return 0;
}

/*
  HWORD
*/
int hword_save(void *val, uchar *str)
{
	int rtn;

	DTPRINTF(0x02, "%s : %04X\n", CFGTYPE_HWORD, *(short *)val);

	rtn = tsnprintf((char *)str, MAXCONFBUF, "%04X", (int)(*(unsigned short *)val));

	return rtn;
}

int hword_load(void *val, uchar *str)
{
	DTPRINTF(0x02, "%s : %s\n", CFGTYPE_HWORD, (char *)str);

	*(unsigned short *)val = (unsigned short)hstou(str);

	return 0;
}

/*
  INT
*/
int int_save(void *val, uchar *str)
{
	int rtn;

	DTPRINTF(0x02, "%s : %d\n", CFGTYPE_INT, *(int *)val);

	rtn = tsnprintf((char *)str, MAXCONFBUF, "%d", *(int *)val);

	return rtn;
}

int int_load(void *val, uchar *str)
{
	DTPRINTF(0x02, "%s : %s\n", CFGTYPE_INT, (char *)str);

	*(int *)val = dstoi(str);

	return 0;
}

#ifdef GSC_COMP_ENABLE_TCPIP
/*
  IPADDRESS
*/
int ipaddr_save(void *val, uchar *str)
{
	char tmp[IP4ADDR_STRLEN_MAX+1];
	int rtn;

	ip4addr_ntoa_r((const ip4_addr_t *)val, tmp, IP4ADDR_STRLEN_MAX);
	DTPRINTF(0x02, "%s : %s\n", CFGTYPE_IPADDRESS, tmp);

	rtn = tsnprintf((char *)str, MAXCONFBUF, "%s", tmp);

	return rtn;
}

int ipaddr_load(void *val, uchar *str)
{
	ip4_addr_t addr;

	DTPRINTF(0x02, "%s : %s\n", CFGTYPE_IPADDRESS, (char *)str);

	ip4addr_aton((char *)str, &addr);

	*(ip4_addr_t *)val = addr;

	return 0;
}
#endif

struct st_conf_purser {
	char *type;
	int (* save)(void *val, uchar *str);
	int (* load)(void *val, uchar *str);
} conf_purser[] = {
	{ CFGTYPE_BYTE, byte_save, byte_load },
	{ CFGTYPE_HWORD, hword_save, hword_load },
	{ CFGTYPE_INT, int_save, int_load },
#ifdef GSC_COMP_ENABLE_TCPIP
	{ CFGTYPE_IPADDRESS, ipaddr_save, ipaddr_load },
#endif
	{ 0, 0, 0 }
};

static int read_line(int fd, char *str, int maxlen)
{
	int len = 0;
	int rtn;
	int i;

	for(i=0; i<(maxlen-1); i++) {
		rtn = read_file(fd, str, 1);
		if(rtn <= 0) {
			*str = 0;
			break;
		}
		if(*str == '\n') {
			str ++;
			len ++;
			*str = 0;
			break;
		}
		str ++;
		len ++;
	}

	return len;
}

static struct st_conf_header *search_name(uchar *name, struct st_conf_header *data)
{
	struct st_conf_header *dp = data;

	DTFPRINTF(0x01, "name = \"%s\"\n", name);

	while(dp->name != 0) {
		//DTPRINTF(0x01, "Check name = %s, type = %s\n", dp->name, dp->type);
		if(strcomp(name, (uchar *)dp->name) == 0) {
			DTFPRINTF(0x01, "Find name = \"%s\"\n", name);
			return dp;
		}
		dp ++;
	}

	return 0;
}

static int purse_line(char *str, uchar **name, uchar **type, uchar **val)
{
	DTFPRINTF(0x01, "[%s]\n", str);

	*name = (uchar *)str;

	*type = strchar(*name, ',');
	if(*type != 0) {
		**type = 0;
		(*type) ++;
		*val = strchar((*type)+1, ',');
		if(*val != 0) {
			**val = 0;
			(*val) ++;
			DTPRINTF(0x01, "name = \"%s\", type = \"%s\", val = \"%s\"\n", *name, *type, *val);
			return 1;
		}
	}

	return 0;
}

int load_appsetting(uchar *fname, struct st_conf_header *data)
{
	int fd;
	int rtn = 0;

	gslog(0, "Load config file \"%s\" start\n", fname);

	fd = open_file(fname, FA_READ);
	if(fd < 0) {
		gslog(0, "Cannot open config file \"%s\"\n", fname);
		return -1;
	}

	do {
		rtn = read_line(fd, confbuf, MAXCONFBUF+1);
		if(rtn > 0) {
			uchar *np;
			uchar *tp;
			uchar *vp;
			confbuf[rtn-1] = 0;
			DTPRINTF(0x01, "[%s]\n", confbuf);

			if(purse_line(confbuf, &np, &tp, &vp) != 0) {
				struct st_conf_header *dp;
				DTPRINTF(0x01, "name = \"%s\", type = \"%s\", val = \"%s\"\n", np, tp, vp);
				dp = search_name(np, data);
				if(dp != 0) {
					struct st_conf_purser *cp = conf_purser;
					DTFPRINTF(0x01, "Find name = \"%s\"\n", dp->name);

					while(cp->load != 0) {
						if(strcomp((uchar *)dp->type, (uchar *)cp->type) == 0) {
							if(strcomp((uchar *)dp->type, tp) == 0) {
								DTFPRINTF(0x01, "Some type name = \"%s\", type = \"%s\"\n", dp->name, tp);
								if(cp->load != 0) {
									int res = 0;
									res = cp->load(dp->val, vp);
									if(res != 0) {
										eprintf("Config load item \"%s\" error(%s)\n", np, vp);
									}
								}
							} else {
								gslog(0, "Config item type different. name = \"%s\", type = \"%s\", type = \"%s\"\n",
								      dp->name, tp, cp->type);
							}
						}
						cp ++;
					}
				}
			}
		}
	} while(rtn > 0);

	close_file(fd);
	gslog(0, "Load config file \"%s\" done\n", fname);
	
	return 0;
}

int save_appsetting(uchar *fname, struct st_conf_header *data)
{
	int fd;
	struct st_conf_header *dp = data;

	gslog(1, "Save config file \"%s\" start\n", fname);

	fd = open_file(fname, FA_WRITE | FA_CREATE_ALWAYS);
	if(fd < 0) {
		gslog(0, "Cannot open config file \"%s\"\n", fname);
		return -1;
	}

	while(dp->type != 0) {
		struct st_conf_purser *cp = conf_purser;
		DTPRINTF(0x01, "name = %s, type = %s, data=%p\n", dp->name, dp->type, dp->val);

		while(cp->save != 0) {
			if(strcomp((uchar *)dp->type, (uchar *)cp->type) == 0) {
				if(cp->save != 0) {
					uchar tmp[MAXCONFBUF+1];
					cp->save(dp->val, tmp);
					int len = tsnprintf(confbuf, MAXCONFBUF, "%s,%s,%s\n", dp->name, dp->type, tmp);
					if(len > 0) {
						int rtn;
						DTPRINTF(0x01, "len = %d, %s", len, confbuf);
						rtn = write_file(fd, confbuf, len);
						if(rtn < 0) {
							eprintf("Config file \"%s\" save error\n", fname);
							close_file(fd);
							return -1;
						}
					}
				}
			}
			cp ++;
		}
		dp ++;
	}

	close_file(fd);
	gslog(1, "Save config file \"%s\" done\n", fname);
	
	return 0;
}

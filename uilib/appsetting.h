/** @file
    @brief	アプリケーション設定保存、復旧

    @date	2019.01.19
    @auther	Takashi SHUDO
*/

#ifndef APPSETTING_H
#define APPSETTING_H

#include "str.h"

#define CFGTYPE_BYTE	"BYTE"
#define CFGTYPE_HWORD	"HWORD"
#define CFGTYPE_INT	"INT"
#define CFGTYPE_UINT	"UINT"
#define CFGTYPE_IPADDRESS	"IPADDRESS"

struct st_conf_header {
	char *name;
	char *type;
	void *val;
};

int load_appsetting(uchar *fname, struct st_conf_header *data);
int save_appsetting(uchar *fname, struct st_conf_header *data);

#endif // APPSETTING_H

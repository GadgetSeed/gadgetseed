/** @file
    @brief	バッチ処理

    GadgetSeed は内臓 shell のコマンドをバッチ処理で実行することが出来ます。

    @date	2009.01.03
    @author	Takashi SHUDO
*/

#include "sysconfig.h"
#include "file.h"
#include "tprintf.h"
#include "shell.h"

//#define DEBUGKBITS 0x03
#include "dkprintf.h"


extern struct st_shell gs_shell;

static int read_line(int fd, uchar *str)
{
	int rt, len = 0;

	while(1) {
		rt = read_file(fd, str, 1);
		if(rt <= 0) {
			goto end;
		}
		DKPRINTF(0x01, "(%02X)", *str);
		if(len > GSC_SHELL_MAX_LINE_COLUMS) {
			goto end;
		}
		if(*str == '\n') {
			goto end;
		}
		if(*str != '\r') {
			len ++;
			str ++;
		}
	}
end:
	*str = 0;

	return len;
}

/**
    @brief	バッチファイルを実行する

    @param[in]	 path	バッチファイルのパス

    @return	実行結果の戻り値
*/
int exec_batch(uchar *path)
{
	int fd;
	int fr;
	int res = 0;
	uchar cmd[GSC_SHELL_MAX_LINE_COLUMS+1];

	fd = open_file(path, FA_READ);
	if(fd < 0) {
		return -1;
	}

	while(1) {
		fr = read_line(fd, cmd);
		DKPRINTF(0x01, "### %ld\n", fr);
		if(fr == 0) {
			res = 0;
			goto error;
		} else if(fr < 0) {
			res = -1;
			goto error;
		}
		DKPRINTF(0x01, "###[%s]\n", cmd);

		res = exec_shell_command(&gs_shell, cmd);
		if(res < 0) {
			goto error;
		}
	};

error:
	close_file(fd);

	return res;
}

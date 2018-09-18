/** @file
    @brief	ファイル操作

    @date	2017.05.02
    @author	Takashi SHUDO
*/

#include "str.h"
#include "shell.h"
#include "tprintf.h"
#include "file.h"


static const struct st_file_operation * const init_file_operation[] = {
	0
};

static const struct st_file_operation * const * p_file_operation = init_file_operation;

void set_file_operation(const struct st_file_operation * const fileop[])
{
	p_file_operation = fileop;
}

#define EXT_LEN 12

int do_file_operation(uchar *fname, uchar *arg)
{
	int rt = 0;
	uchar ext[EXT_LEN + 1] = {0};
	const struct st_file_operation * const *fo = p_file_operation;

	(void)get_filename_extension(ext, fname, EXT_LEN);
	(void)str2cap(ext);

	while(*fo) {
		if(strcomp((uchar *)(*fo)->ext, ext) == 0) {
			rt = (*fo)->operation(fname, arg);
		}
		fo ++;
	}

	return rt;
}

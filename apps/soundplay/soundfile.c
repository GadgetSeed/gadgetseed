/** @file
    @brief	音声ファイル操作

    @date	2017.02.12
    @auther	Takashi SHUDO
*/

#include "soundfile.h"

#include "tprintf.h"
#include "charcode.h"

#define GSLOG_PREFIX	"SFL: "
#include "log.h"
#define SFLLOGLVL	9


//#define DEBUGTBITS 0x02
#include "dtprintf.h"

unsigned char comp_audio_data[MAX_FILEBUF];	// 圧縮音声データ
unsigned char comp_audio_file_name[FF_MAX_LFN + 1];	// UTF-8ファイル名

static int audio_fd = -1;

int soundfile_open(unsigned char *fname)
{
	DTFPRINTF(0x02, "%s\n", fname);

	if(audio_fd < 0) {
		sjisstr_to_utf8str(comp_audio_file_name, fname, FF_MAX_LFN);
		GSLOG(SFLLOGLVL, "Open sound file \"%s\"\n", comp_audio_file_name);

		audio_fd = open_file(fname, FA_READ);
		if(audio_fd < 0) {
			eprintf("File open error %s %d\n", fname, audio_fd);
		}
	} else {
		eprintf("%s: sound file allready open %s %d\n", __FUNCTION__, comp_audio_file_name, audio_fd);
		return -1;
	}

	DTFPRINTF(0x02, "fd = %d\n", audio_fd);

	return audio_fd;
}

int soundfile_read(unsigned char *buf, int size)
{
	int rsize;

	DTFPRINTF(0x01, "fd = %d, buf = %p, size = %d\n", audio_fd, buf, size);

	if(audio_fd < 0) {
		eprintf("%s: sound file not opened\n", __FUNCTION__);
		return -1;
	}

	rsize = read_file(audio_fd, buf, size);

	return rsize;
}

int soundfile_seekcur(int pos)
{
	DTFPRINTF(0x02, "fd = %d, size = %d\n", audio_fd, pos);

	if(audio_fd < 0) {
		eprintf("%s: sound file not opened\n", __FUNCTION__);
		return -1;
	}

	seek_file(audio_fd, pos, SEEK_CUR);

	return pos;
}

int soundfile_seekset(int pos)
{
	DTFPRINTF(0x02, "fd = %d, pos = %d\n", audio_fd, pos);

	if(audio_fd < 0) {
		eprintf("%s: sound file not opened\n", __FUNCTION__);
		return -1;
	}

	seek_file(audio_fd, pos, SEEK_SET);

	return pos;
}

int soundfile_tell(void)
{
	int offset;

	DTFPRINTF(0x02,"audio_fd = %d\n", audio_fd);

	if(audio_fd < 0) {
		eprintf("%s: sound file not opened\n", __FUNCTION__);
		return -1;
	}

	offset = tell_file(audio_fd);

	return offset;
}

int soundfile_size(void)
{
	int fsize;

	DTFPRINTF(0x02,"audio_fd = %d\n", audio_fd);

	if(audio_fd < 0) {
		eprintf("%s: sound file not opened\n", __FUNCTION__);
		return -1;
	}

	fsize = size_file(audio_fd);

	return fsize;
}

void soundfile_close(void)
{
	DTFPRINTF(0x02,"audio_fd = %d\n", audio_fd);

	if(audio_fd >= 0) {
		close_file(audio_fd);
		audio_fd = -1;
		GSLOG(SFLLOGLVL, "Close sound file \"%s\"\n", comp_audio_file_name);
	} else {
		eprintf("%s: sound file not opened\n", __FUNCTION__);
	}
}

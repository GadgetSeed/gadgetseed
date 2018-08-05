/** @file
    @brief	WAVファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#include "sysconfig.h"
#include "tprintf.h"
#include "shell.h"
#include "file.h"
#include "str.h"
#include "ff.h"
#include "soundplay.h"
#include "charcode.h"

#define WAV_FRAME_SIZE	(1024*4)

extern int soundplay_status;

static int wav_play_proc(void)
{
	long rsize = 0;

	while(soundplay_status != SOUND_STOP) {
		rsize = soundplay_readfile(file_buf, WAV_FRAME_SIZE);
		//tprintf("\nR:%ld\n",rsize);
		if(rsize == 0) {
			goto end;
		} else {
			soundplay_write_audiobuf(file_buf, rsize);
		}
	}
end:

	soundplay_stop_sound();

	return 1;
}

static int wav_analyze(int argc, uchar *argv[])
{
	if(argc < 2) {
		tprintf("Usage: wav <filename>\n");
		return 0;
	}

	return 0;
}

const struct st_shell_command com_wav_analyze = {
	"wav", 0, wav_analyze, 0, "Analyze WAV Data", 0
};

static int wav_play(int argc, uchar *argv[])
{
	unsigned char *fname;
	long rsize;
	int rtn = 0;
	long rate;

	if(argc < 2) {
		tprintf("Usage: wav <filename>\n");
		return 0;
	}

	soundplay_stop_play();

	soundplay_init_time();

	fname = (unsigned char *)argv[1];
	sjisstr_to_utf8str(cname, fname, FF_MAX_LFN);
	rtn = soundplay_openfile(fname);
	if(rtn < 0) {
		tprintf("Cannot open \"%s\"\n", cname);
		return 0;
	}

	// 適当なWAVヘッダデコード
	rsize = soundplay_readfile(file_buf, 44);	// WAVのヘッダ分
	if(rsize < 44) {
		tprintf("Header Read Error(size: %ld)\n", rsize);
	}
	rate = ((long)file_buf[24] << 0) +
			((long)file_buf[25] <<  8) +
			((long)file_buf[26] << 16) +
			((long)file_buf[27] << 24);
	tprintf("Sampling Rate : %ld\n", rate);

	soundplay_set_audiobuf_size(WAV_FRAME_SIZE * 2);
	soundplay_set_smprate(rate);

	rsize = soundplay_readfile(file_buf, WAV_FRAME_SIZE);
	soundplay_write_audiobuf(file_buf, rsize);

	tprintf("Start Play \"%s\"\n", cname);
	soundplay_start_sound();
	soundplay_start_proc(wav_play_proc);

	return 0;
}

const struct st_shell_command com_wav_play = {
	"wav", 0, wav_play, 0, "Play WAV Data", 0
};

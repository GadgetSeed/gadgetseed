/** @file
    @brief	MP3ファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#include "tprintf.h"
#include "shell.h"
#include "file.h"
#include "str.h"
#include "sysevent.h"
#include "soundplay.h"
#include "soundio.h"
#include "soundfile.h"
#include "id3tag.h"
#include "mad.h"
#include "mp3play.h"
#include "task/syscall.h"
#include "charcode.h"

#define GSLOG_PREFIX	"MP3: "
#include "log.h"

//#define DEBUGTBITS 0x04
#include "dtprintf.h"


#ifdef DEBUGTBITS
#define STR(var) #var
static const char sound_stat_name[MAX_SOUND_STAT][20] = {
	STR(SOUND_STAT_NOTREADY),
	STR(SOUND_STAT_READY),
	STR(SOUND_STAT_SYNCING),
	STR(SOUND_STAT_PLAYING),
	STR(SOUND_STAT_END)
};
#define PRINT_MP3_STAT()	DTFPRINTF(0x02, "mp3_stat: %s\n", sound_stat_name[soundplay_status])
#else
#define PRINT_MP3_STAT()
#endif


static struct mad_decoder mad_dec;
extern struct st_music_info music_info;
#define MP3BUF_MARGINE	8

static int flg_abort = 0;
static int flg_stream = 0;
static int flg_audioset = 0;

static enum mad_flow mp3_input(void *data, struct mad_stream *stream);
static enum mad_flow mp3_output(void *data, struct mad_header const *header,
				struct mad_pcm *pcm);
static enum mad_flow mp3_error(void *data, struct mad_stream *stream,
			       struct mad_frame *frame);


static enum mad_flow proc_mp3_syncing(void *data, struct mad_stream *stream);

static unsigned char get_sound_event(void)
{
	int rt;
	unsigned char event;

	rt = read_fifo(&soundplay_event, &event, 1);
	if(rt == 0) {
		return SOUND_EVENT_NOEVENT;
	} else {
		return event;
	}
}

static enum mad_flow proc_mp3_ready(void *data, struct mad_stream *stream)
{
	unsigned char event;

	event = get_sound_event();

	switch(event) {
	case SOUND_EVENT_NOEVENT:
		task_sleep(10);
		break;

	case SOUND_EVENT_OPEN:
		PRINT_MP3_STAT();
		break;

	case SOUND_EVENT_PLAY:
		soundplay_status = SOUND_STAT_PLAYING;
		PRINT_MP3_STAT();
		if(audio_frame_count == 0) {
			return proc_mp3_syncing(data, stream);
		} else {
			soundio_start_sound();
		}
		break;

	case SOUND_EVENT_STOP:
		soundplay_status = SOUND_STAT_END;
		PRINT_MP3_STAT();
		flg_abort = 1;
		DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
		break;

	case SOUND_EVENT_PAUSE:
		task_sleep(10);
		break;

	case SOUND_EVENT_SYNC:
		soundplay_status = SOUND_STAT_SYNCING;
		PRINT_MP3_STAT();
		break;

	case SOUND_EVENT_CLOSE:
		soundplay_status = SOUND_STAT_NOTREADY;
		PRINT_MP3_STAT();
		break;

	default:
		break;
	}

	return MAD_FLOW_CONTINUE;
}

static enum mad_flow proc_mp3_syncing(void *data, struct mad_stream *stream)
{
	int rtn = 0;
	int flg_find_header = 0;
	int i;

	// ヘッダ分読み込み
	rtn = soundfile_read(comp_audio_data, MPEG_FRAME_HEADER_SIZE);
	if(rtn != MPEG_FRAME_HEADER_SIZE) {
		eprintf("Frame header read error(%d)\n", rtn);
		DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
	}
	XDUMP(0x07, comp_audio_data, MPEG_FRAME_HEADER_SIZE);
	GSLOG(0, "Syncing MP3 Frame\n");

	do {
		rtn = mpeg_frame_header_decode(&music_info, comp_audio_data);
		if(rtn != 0) {
			// 同期データではない
			DTPRINTF(0x07, "Searching MP3 Frame header\n");
			XDUMP(0x08, comp_audio_data, MPEG_FRAME_HEADER_SIZE);
			for(i=0; i<(MPEG_FRAME_HEADER_SIZE-1); i++) {
				comp_audio_data[i] = comp_audio_data[i+1];
			}
			rtn = soundfile_read(&comp_audio_data[MPEG_FRAME_HEADER_SIZE-1], 1);
			DTPRINTF(0x07, "soundplay_readfile rtn = %d\n", rtn);
			if(rtn == 0) {
				// EOF
				DTPRINTF(0x07, "MP3 data EOF(%d)\n", rtn);
				soundplay_status = SOUND_STAT_END;
				PRINT_MP3_STAT();
				DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
				return MAD_FLOW_STOP;
			} else if(rtn < 0) {
				// Read Timeout
				DTPRINTF(0x07, "MP3 Frame header read timeout(%d)\n", rtn);
				if(flg_stream == 0) {
					soundplay_status = SOUND_STAT_END;
					PRINT_MP3_STAT();
				}
			}
		} else {
			// 同期
			GSLOG(0, "Synced MP3 Frame\n");
			flg_find_header = 1;
			rtn = soundfile_read(&comp_audio_data[MPEG_FRAME_HEADER_SIZE],
					     music_info.frame_length - MPEG_FRAME_HEADER_SIZE + MP3BUF_MARGINE);
			DTPRINTF(0x07, "soundfile_readfile rtn = %d\n", rtn);
			XDUMP(0x08, comp_audio_data, 16/*music_info.frame_length*/);
			if(rtn == 0) {
				// EOF
				GSLOG(0, "MP3 data EOF(%d)\n", rtn);
				DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
				return MAD_FLOW_STOP;
			} else if(rtn < 0) {
				// Read Timeout
				GSLOG(0, "MP3 Frame header read timeout(%d)\n", rtn);
				if(flg_stream == 0) {
					soundplay_status = SOUND_STAT_END;
					PRINT_MP3_STAT();
				}
			} else {
				int stsize = rtn + MP3BUF_MARGINE;
				DTPRINTF(0x01, "%4d : Streem Size : %d\n", audio_frame_count, stsize);
				GSLOG(0, "Resync success\n");
				mad_stream_buffer(stream, comp_audio_data, stsize);
				soundplay_status = SOUND_STAT_PLAYING;
				PRINT_MP3_STAT();
				return MAD_FLOW_CONTINUE;
			}
		}
	} while(flg_find_header == 0);

	DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
	return MAD_FLOW_STOP;
}

static enum mad_flow proc_mp3_playing(void *data, struct mad_stream *stream)
{
	int rtn = 0;
	int i;
	unsigned char event;

	event = get_sound_event();

	switch(event) {
	case SOUND_EVENT_NOEVENT:
	case SOUND_EVENT_PLAY:
		break;

	case SOUND_EVENT_STOP:
		DTPRINTF(0x07, "mad input stop\n");
		flg_abort = 1;
		soundplay_status = SOUND_STAT_END;
		PRINT_MP3_STAT();
		DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
		break;

	case SOUND_EVENT_PAUSE:
		soundplay_status = SOUND_STAT_READY;
		soundio_pause_sound();
		PRINT_MP3_STAT();
		return MAD_FLOW_CONTINUE;
		break;

	case SOUND_EVENT_SYNC:
		break;

	default:
		break;
	}

	for(i=0; i<MP3BUF_MARGINE; i++) {
		comp_audio_data[i] = comp_audio_data[music_info.frame_length + i];
	}
	XDUMP(0x08, comp_audio_data, MPEG_FRAME_HEADER_SIZE);
	rtn = mpeg_frame_header_decode(&music_info, comp_audio_data);
	if(rtn != 0) {
		DTPRINTF(0x07, "mpeg_frame_header_decode rtn = %d\n",rtn);
		DTPRINTF(0x07, "Frame Lost\n");
		XDUMP(0x07, comp_audio_data, 16/*music_info.frame_length + MP3BUF_MARGINE*/);
#if 0
		soundplay_status = SOUND_STAT_END;
		PRINT_MP3_STAT();
		DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
#else
		soundplay_status = SOUND_STAT_SYNCING;
		PRINT_MP3_STAT();
		DTFPRINTF(0x07, "return MAD_FLOW_IGNORE\n");
		return MAD_FLOW_IGNORE;
#endif
	}

	DTPRINTF(0x02, "%4d : Frame Size : %d\n", audio_frame_count, music_info.frame_length);
	rtn = soundfile_read(&comp_audio_data[MP3BUF_MARGINE], music_info.frame_length);
	create_event(103, 0, 0); // EVT_IRADIO_RECEIVE
	XDUMP(0x08, comp_audio_data, 16/*music_info.frame_length*/);
	if(rtn == 0) {
		// EOF
		DTPRINTF(0x07, "MP3 data EOF(%d)\n", rtn);
		soundplay_status = SOUND_STAT_END;
		PRINT_MP3_STAT();
		DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
	} else if(rtn < 0) {
		// Read Timeout
		DTPRINTF(0x07, "MP3 Frame header read timeout(%d)\n", rtn);
		if(flg_stream == 0) {
			soundplay_status = SOUND_STAT_END;
			PRINT_MP3_STAT();
		} else {
			soundplay_status = SOUND_STAT_SYNCING;
			PRINT_MP3_STAT();
		}
		DTFPRINTF(0x07, "return MAD_FLOW_IGNORE\n");
		return MAD_FLOW_IGNORE;
	} else {
		int stsize = 0;
		if(flg_stream == 0) {
			// 最終フレームチェック
			int trtn = 0;
			trtn = mpeg_frame_header_check(&comp_audio_data[music_info.frame_length]);
			if(trtn != 0) {
				DTPRINTF(0x07, "Last Frame(%d).\n", audio_frame_count);
				stsize = rtn;
			} else {
				stsize = rtn + MP3BUF_MARGINE;
			}
		} else {
			if(rtn > 0) {
				stsize = rtn + MP3BUF_MARGINE;
			}
		}

		DTPRINTF(0x01, "%4d : Streem Size : %d\n", audio_frame_count, stsize);
		mad_stream_buffer(stream, comp_audio_data, stsize);

		return MAD_FLOW_CONTINUE;
	}
}

/**/

static enum mad_flow mp3_input(void *data, struct mad_stream *stream)
{
	//tprintf(".");

	soundio_mixer_proc();

	if(next_audio_frame_count >= 0) {
		mp3file_seek(next_audio_frame_count);
		audio_frame_count = next_audio_frame_count;
		next_audio_frame_count = -1;
	}

	switch(soundplay_status) {
	case SOUND_STAT_READY:
		return proc_mp3_ready(data, stream);
		break;

	case SOUND_STAT_SYNCING:
		return proc_mp3_syncing(data, stream);
		break;

	case SOUND_STAT_PLAYING:
		return proc_mp3_playing(data, stream);
		break;

	default:
		DTPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
		break;
	}
}


static inline signed int scale(mad_fixed_t sample)
{
	/* round */
	sample += (1L << (MAD_F_FRACBITS - 16));

	/* clip */
	if (sample >= MAD_F_ONE)
		sample = MAD_F_ONE - 1;
	else if (sample < -MAD_F_ONE)
		sample = -MAD_F_ONE;

	/* quantize */
	return sample >> (MAD_F_FRACBITS + 1 - 16);
}

#define WORD_BUF
#ifdef WORD_BUF
static unsigned short audio_buf[1152*2];
#else
static char audio_buf[1152*4];
#endif

static enum mad_flow mp3_output(void *data,
				struct mad_header const *header,
				struct mad_pcm *pcm)
{
	unsigned int nchannels, nsamples;
	mad_fixed_t const *left_ch, *right_ch;
	int i = 0;

	if(soundplay_status == SOUND_STAT_END) {
		flg_abort = 1;
		DTFPRINTF(0x07, "return MAD_FLOW_STOP\n");
		return MAD_FLOW_STOP;
	}

	nchannels = pcm->channels;
	nsamples  = pcm->length;
	left_ch   = pcm->samples[0];
#ifdef GSC_TARGET_SYSTEM_EMU
	right_ch  = pcm->samples[0];	// モノラル(左のみ)
#else
	right_ch  = pcm->samples[1];
#endif

	DTPRINTF(0x01, "CH:%d, LEN:%d\n", (int)nchannels, (int)nsamples);

	if(nchannels == 2) {
		for(i=0; i<nsamples; i++) {
#ifdef WORD_BUF
			audio_buf[i*2+0] = scale(*left_ch);
			audio_buf[i*2+1] = scale(*right_ch);
			left_ch ++;
			right_ch ++;
#ifdef GSC_TARGET_SYSTEM_EMU
			/*
			  何故かemu(Linux)だとMP3(libmad)のデコード結果の音程が低い
			  暫定的な対策(右チャンネルはノイジー)
			 */
			left_ch ++;
			right_ch ++;
#endif
#else
			signed int sample;
			sample = scale(*left_ch++);
			audio_buf[i*4 + 0] = ((sample >> 0) & 0xff);
			audio_buf[i*4 + 1] = ((sample >> 8) & 0xff);
			sample = scale(*right_ch++);
			audio_buf[i*4 + 2] = ((sample >> 0) & 0xff);
			audio_buf[i*4 + 3] = ((sample >> 8) & 0xff);
#endif
		}
	}

	if(audio_frame_count >= 1) {
		if(flg_audioset == 0) {
			int bufsize = nsamples * 2 * 2 * 2;
			tprintf("Audio buffer size = %d\n", bufsize);
			soundio_set_audiobuf_size(bufsize);
			soundio_set_smprate(music_info.sampling_rate);
			flg_audioset = 1;
		}
	}

	soundio_write_audiobuf((unsigned char *)audio_buf, nsamples * 2 * 2);

	if(audio_frame_count >= 2) {
		if(flg_audioset == 1) {
			soundio_start_sound();
			flg_audioset = 2;
		}
	}

	audio_play_time = calc_play_time(&music_info, audio_frame_count);
	disp_play_time(audio_play_time);

#if 0 // TODO
	if(next_audio_frame_count < 0) {
		audio_frame_count ++;
	} else {
		audio_frame_count = next_audio_frame_count;
		next_audio_frame_count = -1;;
	}
#else
	audio_frame_count ++;
#endif
	DTFPRINTF(0x01, "audio_frame_count = %d\n", audio_frame_count);
	return MAD_FLOW_CONTINUE;
}

static enum mad_flow mp3_error(void *data,
			   struct mad_stream *stream,
			   struct mad_frame *frame)
{
	if(stream->error != MAD_ERROR_BADDATAPTR) {
		GSLOG(1, "Frame %d : decoding error 0x%04x (%s) at byte offset %lX\n",
		      audio_frame_count,
		      stream->error, mad_stream_errorstr(stream),
		      (unsigned long)stream->this_frame);
		lxdump(1, data, 16/*music_info.frame_length*/);
	}

	audio_frame_count ++;

	return MAD_FLOW_CONTINUE;
}

static int mp3_play_proc(void)
{
	int rtn = 0;

	while(soundplay_status == SOUND_STAT_READY) {
		unsigned char event;

		event = get_sound_event();

		if(event == SOUND_EVENT_PLAY) {
			soundplay_status = SOUND_STAT_SYNCING;
		} else if(event == SOUND_EVENT_STOP) {
			soundplay_status = SOUND_STAT_END;
			return 1;
		} else {
			task_sleep(10);
		}
	}

loop:
	rtn = mad_decoder_run(&mad_dec, MAD_DECODER_MODE_SYNC);
	GSLOG(0, "mad_decoder_run = %d\n", rtn);
	mad_decoder_finish(&mad_dec);

	if(flg_abort != 0) {
		GSLOG(0, "mp3_play_proc abort\n");
		soundio_stop_sound();
	} else {
#if 1
		if(flg_stream != 0) {
			GSLOG(0, "mp3_play_proc resync\n");
			soundplay_status = SOUND_STAT_SYNCING;
			PRINT_MP3_STAT();
			mad_decoder_init(&mad_dec, comp_audio_data,
					 mp3_input, 0 /* header */, 0 /* filter */, mp3_output,
					 mp3_error, 0 /* message */);
			audio_frame_count = 0;
			goto loop;
		} else {
			GSLOG(0, "mp3_play_proc end\n");
			soundio_end_sound();
		}
#else
		GSLOG(0, "mp3_play_proc end\n");
		soundio_end_sound();
#endif
	}

	return 1;
}

int mp3file_open(uchar *fname)
{
	int rtn = 0;

	rtn = soundfile_open(fname);
	if(rtn < 0) {
		return -1;
	}

	flg_audioset = 0;
	flg_abort = 0;
	audio_frame_count = 0;
	next_audio_frame_count = -1;
	flg_stream = 0;

	rtn = id3tag_decode(&music_info, soundfile_read, soundfile_seekcur, soundfile_seekset, soundfile_size, soundfile_tell);
	if(rtn > 0) {
		disp_music_info(&music_info);
		soundplay_status = SOUND_STAT_READY;
		soundio_prepared_sound();
		soundplay_start_proc(mp3_play_proc);
		mad_decoder_init(&mad_dec, comp_audio_data,
				 mp3_input, 0 /* header */, 0 /* filter */, mp3_output,
				 mp3_error, 0 /* message */);
		DTFPRINTF(0x04, "frame_start = %d, frame_length = %d\n", music_info.frame_start, music_info.frame_length);
	}

	return rtn;
}

int mp3stream_open(uchar *fname, uchar *title)
{
	int rtn = 0;

	rtn = soundfile_open(fname);
	if(rtn < 0) {
		return -1;
	}

	flg_audioset = 0;
	flg_abort = 0;
	audio_frame_count = 0;

	if(title != 0) {
		int bit_rate = 128;
		flg_stream = 1;

		strncopy(music_info.album, title, MAX_MINFO_STR);
		music_info.format = MUSIC_FMT_MP3;
		music_info.frame_size = 1152;	// 取り敢えず固定[TODO]
		music_info.bit_rate = bit_rate;
		music_info.sampling_rate = 44100;
		music_info.mpeg_padding = 1;
		music_info.channel = 1;
		music_info.frame_length = ((144 * music_info.bit_rate * 1000)/(music_info.sampling_rate)) + music_info.mpeg_padding;
		disp_music_info(&music_info);

		mad_decoder_init(&mad_dec, comp_audio_data,
				 mp3_input, 0 /* header */, 0 /* filter */, mp3_output,
				 mp3_error, 0 /* message */);

		soundplay_status = SOUND_STAT_SYNCING;
		PRINT_MP3_STAT();
		soundplay_start_proc(mp3_play_proc);
	} else {
		flg_stream = 0;
		rtn = id3tag_decode(&music_info, soundfile_read, soundfile_seekcur, soundfile_seekset, soundfile_size, soundfile_tell);
		if(rtn > 0) {
			disp_music_info(&music_info);
			soundplay_status = SOUND_STAT_READY;
			soundio_prepared_sound();
			soundplay_start_proc(mp3_play_proc);
			mad_decoder_init(&mad_dec, comp_audio_data,
					 mp3_input, 0 /* header */, 0 /* filter */, mp3_output,
					 mp3_error, 0 /* message */);
		}
	}

	return rtn;
}

void mp3file_seek(int pos)
{
	int new_pos;

	new_pos = music_info.frame_start + (pos * music_info.frame_length);
	DTFPRINTF(0x04, "frame_start = %d, pos = %d, new_pos = %d\n", music_info.frame_start, pos, new_pos);

	soundfile_seekset(new_pos);
}


const struct st_shell_command com_mp3_open;

static int mp3_open(int argc, uchar *argv[])
{
	int rtn = 0;
	uchar *title = 0;

	if(argc < 2) {
		print_command_usage(&com_mp3_open);
		return 0;
	}

	if(argc > 2) {
		title = argv[2];
	}

	rtn = mp3stream_open(argv[1], title);

	return rtn;
}

const struct st_shell_command com_mp3_open = {
	.name		= "mp3",
	.command	= mp3_open,
	.usage_str	= "<file> [title]",
	.manual_str	= "Open MP3 file"
};

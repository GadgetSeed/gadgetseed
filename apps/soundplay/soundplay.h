/** @file
    @brief	音声ファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#ifndef SOUNDPLAY_H
#define SOUNDPLAY_H

#include "file.h"

typedef enum {
	SOUND_EVENT_NOEVENT,
	SOUND_EVENT_OPEN,
	SOUND_EVENT_PLAY,
	SOUND_EVENT_PAUSE,
	SOUND_EVENT_SYNC,
	SOUND_EVENT_STOP,
	SOUND_EVENT_CLOSE,
	MAX_SOUND_EVENT
} sound_event;
extern struct st_fifo soundplay_event;

typedef enum {
	SOUND_STAT_NOTREADY,
	SOUND_STAT_READY,
	SOUND_STAT_SYNCING,
	SOUND_STAT_PLAYING,
	SOUND_STAT_END,
	MAX_SOUND_STAT
} sound_stat;
extern sound_stat soundplay_status;

extern struct st_music_info music_info;

extern int audio_frame_count;
extern int next_audio_frame_count;
extern unsigned int audio_play_time;

int soundplay_open(uchar *fname);
void soundplay_start_proc(int (* func)(void));
void soundplay_play(void);
void soundplay_pause(void);
void soundplay_resync(void);
void soundplay_stop(void);
void soundplay_close(void);
void soundplay_seek(int pos);
int soundplay_playbackpos(void);
void soundplay_volume(int volume);
void soundplay_artwork(int flg);

#endif // SOUNDPLAY_H

/** @file
    @brief	音声ファイル再生

    @date	2017.03.26
    @auther	Takashi SHUDO
*/

#ifndef SOUNDPLAY_H
#define SOUNDPLAY_H

#define MAX_FILEBUF	(1024*10)

#define SOUND_STOP	0
#define SOUND_PLAY	1
#define SOUND_PAUSE	2
extern int soundplay_status;
extern int next_soundplay_status;

extern unsigned char file_buf[MAX_FILEBUF];
extern unsigned char cname[FF_MAX_LFN + 1];
extern int audio_buf_size;
extern int audio_frame_count;
extern int next_audio_frame_count;
extern unsigned int audio_play_time;

void soundplay_mixer_proc(void);
void soundplay_prepare_sound(void);
void soundplay_start_sound(void);
void soundplay_cotinue_sound(void);
void soundplay_end_sound(void);
void soundplay_stop_sound(void);
void soundplay_pause_sound(void);
int soundplay_set_smprate(int rate);
int soundplay_set_audiobuf_size(int size);
int soundplay_write_audiobuf(unsigned char *buf, int size);
void soundplay_wait_audiobuf(unsigned char **p_buf);

int soundplay_openfile(unsigned char *fname);
int soundplay_readfile(unsigned char *buf, int size);
int soundplay_seekfile(unsigned char *buf, int size);
int soundplay_seeksetfile(int pos);
int soundplay_tellfile(void);
void soundplay_closefile(void);

void soundplay_start_proc(int (* func)(void));
void soundplay_stop_play(void);
void soundplay_pause_play(void);
void soundplay_continue_play(void);
void soundplay_move_play(int pos);
void soundplay_init_time(void);

#endif // SOUNDPLAY_H

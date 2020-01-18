/** @file
    @brief	AUDIOデバイス操作

    @date	2017.02.12
    @auther	Takashi SHUDO
*/

#ifndef SOUNDIO_H
#define SOUNDIO_H

extern int audio_buf_size;

void init_soundio(void);
int soundio_get_volume(void);
void soundio_set_volume(int vol);
void soundio_mixer_proc(void);
void soundio_prepared_sound(void);
void soundio_start_sound(void);
void soundio_cotinue_sound(void);
void soundio_end_sound(void);
void soundio_stop_sound(void);
void soundio_pause_sound(void);
int soundio_set_smprate(int rate);
int soundio_set_audiobuf_size(int size);
int soundio_write_audiobuf(unsigned char *buf, int size);
void soundio_wait_audiobuf(unsigned char **p_buf);

#endif // SOUNDIO_H

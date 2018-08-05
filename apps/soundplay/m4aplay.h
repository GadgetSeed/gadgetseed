/** @file
    @brief	M4A(AAC)ファイル再生

    @date	2017.04.08
    @auther	Takashi SHUDO
*/

#ifndef M4APLAY_H
#define M4APLAY_H

int m4afile_analyze(struct st_music_info *info, unsigned char *fname);

extern const struct st_shell_command com_m4a_analyze;
extern const struct st_shell_command com_m4a_play;

#endif // M4APLAY_H

/** @file
    @brief	スペクトラム・アナライザ

    @date	2017.07.09
    @auther	Takashi SHUDO
*/

#ifndef SPECTRUM_ANALYZER_H
#define SPECTRUM_ANALYZER_H

#if 1
#define SPA_SMP_NUM	512
#else
#define SPA_SMP_NUM	1024
#endif
#define SPA_ANA_SMP	(SPA_SMP_NUM/2)

struct st_audio_spectrum {
	unsigned long frame_num;
	unsigned char spectrum[SPA_ANA_SMP * 2];
};

void proc_spectrum_analyze(unsigned char *spectrum, short *buf);
void init_window_table(void);

#endif // SPECTRUM_ANALYZER_H

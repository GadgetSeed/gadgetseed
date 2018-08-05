/** @file
    @brief	スペクトラム・アナライザ表示

    @date	2017.07.23
    @auther	Takashi SHUDO
*/

#ifndef SPECTRUM_VIEW_H
#define SPECTRUM_VIEW_H

#include "../soundplay/spectrum_analyzer.h"

void draw_spectrum(struct st_audio_spectrum *asp);

#endif // SPECTRUM_VIEW_H

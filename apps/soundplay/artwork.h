/** @file
    @brief	ID3,MP4アートワーク画像処理

    @date	2019.12.16
    @auther	Takashi SHUDO
*/

#ifndef ARTWORK_H
#define ARTWORK_H

#include "music_info.h"

void decode_jpeg_artwork(struct st_music_info *info, void *jpegdata);
void decode_png_artwork(struct st_music_info *info, void *pngdata);
void decode_jpeg_artwork_bg(struct st_music_info *info, void *data, void *freemem);
void decode_png_artwork_bg(struct st_music_info *info, void *data, void *freemem);

#endif // ARTWORK_H

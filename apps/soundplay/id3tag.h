/** @file
    @brief	ID3TAGデコード

    @date	2017.03.27
    @auther	Takashi SHUDO
*/

#ifndef ID3TAG_H
#define ID3TAG_H

#include "music_info.h"

#define ID3TAG_HEADER_SIZE		(10)
#define ID3TAG_FRAME_HEADER_SIZE	(10)
#define MPEG_FRAME_HEADER_SIZE		(4)

void set_id3_decode_artwork(int flg_env);
int id3tag_decode(struct st_music_info *info, tag_read_func tag_read, tag_seekcur_func tag_seekcur, tag_seekset_func tag_seekset,
		  tag_size_func tag_size, tag_tell_func tag_tell);
unsigned short mpeg_frame_header_check(unsigned char *data);
int mpeg_frame_header_decode(struct st_music_info *info, unsigned char *data);

#endif // ID3TAG_H

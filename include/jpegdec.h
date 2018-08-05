/** @file
    @brief	JPEGデコード

    @date	2017.11.12
    @author	Takashi SHUDO
*/

#ifndef JPEGDEC_H
#define JPEGDEC_H

#include "picojpeg-master/picojpeg.h"

int get_jpeg_file_info(int fd, pjpeg_image_info_t *jpeginfo, int reduce);
int get_jpeg_data_info(unsigned char *jpeg_data, pjpeg_image_info_t *jpeginfo, int reduce);
int draw_jpeg(short px, short py);
int decode_jpeg(void *image);

#endif // JPEGDEC_H

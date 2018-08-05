/** @file
    @brief	PNGデコード

    @date	2018.01.03
    @author	Takashi SHUDO
*/

#ifndef PNGDEC_H
#define PNGDEC_H

int get_png_file_info(int fd, short *png_width, short *png_height);
int get_png_data_info(unsigned char *data, short *png_width, short *png_height);
int decode_png(void *image);
void dispose_png_info(void);

#endif // PNGDEC_H

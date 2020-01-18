/** @file
    @brief	PNGデコード

    libpng を使用
    http://www.libpng.org/

    @date	2018.01.03
    @author	Takashi SHUDO
*/

#include "file.h"
#include "pngdec.h"
#include "str.h"
#include "memory.h"
#include "graphics.h"
#include "png.h"

//#define DEBUGTBITS 0x01
#include "dtprintf.h"


static png_structp png_ptr = 0;
static png_infop info_ptr = 0;

/*
 * ファイル上データデコード
 */
typedef struct my_png_file_ {
	int fd;
} my_png_file;


static void png_fileread_func(png_structp png_sp, png_bytep buf, png_size_t size)
{
	my_png_file *png_file;

	DTFPRINTF(0x02, "size = %d\n", size);

	png_file = (my_png_file *)png_get_io_ptr(png_sp);

	read_file(png_file->fd, buf, size);

	XDUMP(0x02, buf, size);
}

static my_png_file png_file;

int get_png_file_info(int fd, short *png_width, short *png_height)
{
	int res = 0;
#if 0	// png_read_info()で同様の処理は行っているので
	unsigned char png_sig[8];
	int rt;
	rt = read_file(fd, png_sig, 8);
	seek_file(fd, 0, SEEK_SET);
	if(rt != 8) {
		DTPRINTF(0x01, "PNG file read error\n");
		return -1;
	}

	rt = png_check_sig(png_sig, 8);
	if(rt == 0) {
		DTPRINTF(0x01, "png_check_sig() error\n");
		return -1;
	}
#endif

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == 0) {
		DTFPRINTF(0x01, "png_create_read_struct() error\n");
		res = -1;
		goto error;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		DTFPRINTF(0x01, "png_create_info_struct() error\n");
		res = -1;
		goto error;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		DTFPRINTF(0x01, "decode error\n");
		res = -1;
		goto error;
	}

	png_file.fd = fd;
	png_set_read_fn(png_ptr, (png_voidp)&png_file, (png_rw_ptr)png_fileread_func);

	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	int compression_type, filter_type;

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,&color_type,
		     &interlace_type, &compression_type, &filter_type);
	DTPRINTF(0x01, "width = %u, height = %u, bpp = %d, color_type = %d\n",
		 width, height, bit_depth, color_type);

	*png_width = width;
	*png_height = height;

	return res;

error:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return res;
}


/*
 * メモリ上データデコード
 */
struct my_png_data {
	unsigned char *data_ptr;
};


static void png_dataread_func(png_structp png_ptr, png_bytep buf, png_size_t size)
{
	DTFPRINTF(0x02, "size = %d\n", size);

	struct my_png_data *png_data = (struct my_png_data *)png_get_io_ptr(png_ptr);

	memorycopy(buf, png_data->data_ptr, size);
	png_data->data_ptr += size;

	XDUMP(0x02, buf, size);
}

static struct my_png_data png_data;

int get_png_data_info(unsigned char *data, short *png_width, short *png_height)
{
	int res = 0;
#if 0	// png_read_info()で同様の処理は行っているので
	unsigned char png_sig[8];
	int rt;
	rt = read_file(fd, png_sig, 8);
	seek_file(fd, 0, SEEK_SET);
	if(rt != 8) {
		DTPRINTF(0x01, "PNG file read error\n");
		return -1;
	}

	rt = png_check_sig(png_sig, 8);
	if(rt == 0) {
		DTPRINTF(0x01, "png_check_sig() error\n");
		return -1;
	}
#endif

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == 0) {
		DTFPRINTF(0x01, "png_create_read_struct() error\n");
		res = -1;
		goto error;
	}

	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL) {
		DTFPRINTF(0x01, "png_create_info_struct() error\n");
		res = -1;
		goto error;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		DTFPRINTF(0x01, "decode error\n");
		res = -1;
		goto error;
	}

	png_data.data_ptr = data;
	png_set_read_fn(png_ptr, (png_voidp)&png_data, (png_rw_ptr)png_dataread_func);

	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	int compression_type, filter_type;

	png_read_info(png_ptr, info_ptr);
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth,&color_type,
		     &interlace_type, &compression_type, &filter_type);
	DTPRINTF(0x01, "width = %u, height = %u bpp = %d color_type = %d\n",
		 width, height, bit_depth, color_type);

	*png_width = width;
	*png_height = height;

	return res;

error:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return res;
}


int decode_png(void *image)
{
	int res = 0;
	unsigned char **rowimage;
	int i, j;
	int rowbytes;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;
	int compression_type, filter_type;
	PIXEL_DATA *pixel_p = image;

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
		     &interlace_type, &compression_type, &filter_type);

	switch(color_type) {
	case PNG_COLOR_TYPE_PALETTE:  // インデックスカラー
		DTPRINTF(0x01, "PNG_COLOR_TYPE_PALETTE not supported\n");
		res = -1;
		goto end;
		break;

	case PNG_COLOR_TYPE_GRAY:  // グレースケール
		DTPRINTF(0x01, "PNG_COLOR_TYPE_GRAY not supported\n");
		res = -1;
		break;

	case PNG_COLOR_TYPE_GRAY_ALPHA: // グレースケール+α
		DTPRINTF(0x01, "PNG_COLOR_TYPE_GRAY_ALPHA not supported\n");
		res = -1;
		goto end;
		break;

	case PNG_COLOR_TYPE_RGB:  // RGB
		DTPRINTF(0x01, "PNG_COLOR_TYPE_RGB\n");
		break;

	case PNG_COLOR_TYPE_RGB_ALPHA:  // RGBA
		DTPRINTF(0x01, "PNG_COLOR_TYPE_RGB_ALPHA\n");
		break;

	default:
		DTPRINTF(0x01, "Unknown type(%d)\n", color_type);
		res = -1;
		goto end;
		break;
	}

	if(setjmp(png_jmpbuf(png_ptr))) {
		DTFPRINTF(0x01, "decode error\n");
		res = -1;
		goto error;
	}

	rowimage = (png_bytepp)alloc_memory(height * sizeof(png_bytep));
	if(rowimage == 0) {
		res = -1;
		goto end;
	}
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	DTPRINTF(0x01, "rowbytes = %d\n", rowbytes);
	for(i=0; i<height; i++) {
		rowimage[i] = (png_bytep)alloc_memory(rowbytes);
		if(rowimage[i] == 0) {
			int j;
			for(j=0; j<=i; j++) {
				free_memory(rowimage[i]);
			}
			free_memory(rowimage);
			res = -1;
			goto end;
		}
	}

	png_read_image(png_ptr, rowimage);

	for(i=0; i<height; i++) {
		XDUMP(0x02, rowimage[i], width);
		for(j=0; j<width; j++) {
			switch(color_type) {
			case PNG_COLOR_TYPE_RGB_ALPHA:
				pixel_p[width*i + j] = RGB(rowimage[i][j*4+0], rowimage[i][j*4+1], rowimage[i][j*4+2]);
				break;

			case PNG_COLOR_TYPE_RGB:
				pixel_p[width*i + j] = RGB(rowimage[i][j*3+0], rowimage[i][j*3+1], rowimage[i][j*3+2]);
				break;

			default:
				break;
			}
		}
	}
	//draw_image(px, py, width, height, image, width);

error:
	for(i=0; i<height; i++) {
		free_memory(rowimage[i]);
	}
	free_memory(rowimage);

end:
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	return res;
}


void dispose_png_info(void)
{
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
}

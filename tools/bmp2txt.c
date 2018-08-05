/*
  BITMAP変換ツール

  2007.07.25 Takashi SHUDO
*/

#include <stdio.h>
#include <stdlib.h>

//#define DEBUG

#define SIZEOF_BITMAPFILEHEADER	14
typedef struct tagBITMAPFILEHEADER {
#define DPOS_bfType	0
#define DSIZ_bfType	2
	unsigned short bfType;
#define DPOS_bfSize	2
#define DSIZ_bfSize	4
	unsigned long  bfSize;
	unsigned short bfReserved1;
	unsigned short bfReserved2;
#define DPOS_bfOffBits	10
#define DSIZ_bfOffBits	4
	unsigned long  bfOffBits;
} __attribute__((packed)) BITMAPFILEHEADER;

#define SIZEOF_BITMAPINFOHEADER	40
typedef struct tagBITMAPINFOHEADER{
	unsigned long  biSize;
#define DPOS_biWidth	4
#define DSIZ_biWidth	4
	long           biWidth;
#define DPOS_biHeight	8
#define DSIZ_biHeight	4
	long           biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned long  biCompression;
	unsigned long  biSizeImage;
	long           biXPixPerMeter;
	long           biYPixPerMeter;
	unsigned long  biClrUsed;
	unsigned long  biClrImporant;
} __attribute__((packed)) BITMAPINFOHEADER;

BITMAPFILEHEADER	freader;
BITMAPINFOHEADER	infohead;
unsigned char *gdata;
long gsize;
long lsize;

static unsigned long get_val(unsigned char *buf, unsigned long pos,
			     unsigned long size)
{
	unsigned long val = 0;

	switch(size) {
	case 2:
		val =
				((unsigned long)buf[pos+1]<<8) +
				((unsigned long)buf[pos+2]);
		break;
		
	case 4:
		val =
				((unsigned long)buf[pos+3]<<24) +
				((unsigned long)buf[pos+2]<<16) +
				((unsigned long)buf[pos+1]<<8) +
				((unsigned long)buf[pos+0]);
		break;
		
	default:
		fprintf(stderr, "get_val error size = %ld\n", size);
		break;
	}

	return val;
}

int load_bmpheader(FILE *fp, char *name)
{
	size_t sz;
	unsigned char buf[SIZEOF_BITMAPINFOHEADER];
	
	sz = fread(buf, SIZEOF_BITMAPFILEHEADER, 1, fp);
	if(sz != 1) {
		fprintf(stderr, "Cannot read Header.(%zd)\n", sz);
		return -1;
	}
	
	freader.bfSize		= get_val(buf, DPOS_bfSize, DSIZ_bfSize);
	freader.bfOffBits	= get_val(buf, DPOS_bfOffBits, DSIZ_bfOffBits);
#ifdef DEBUG
	fprintf(stderr, "%c%c\n", (freader.bfType>>8), freader.bfType & 0xff);
	fprintf(stderr, "freader.bfSize = %d\n", (int)freader.bfSize);
	fprintf(stderr, "freader.bfReserved1 = %04x\n", freader.bfReserved1);
	fprintf(stderr, "freader.bfReserved2 = %04x\n", freader.bfReserved2);
	fprintf(stderr, "freader.bfOffBits = %d\n", (int)freader.bfOffBits);
#endif
			
	sz = fread(buf, SIZEOF_BITMAPINFOHEADER, 1, fp);
	if(sz != 1) {
		fprintf(stderr, "Cannot read InfoHeader.(%zd)\n", sz);
		return -1;
	}

	infohead.biWidth	= get_val(buf, DPOS_biWidth, DSIZ_biWidth);
	infohead.biHeight	= get_val(buf, DPOS_biHeight, DSIZ_biHeight);

#ifdef DEBUG
	fprintf(stderr, "infohead.biSize = %d\n", (int)infohead.biSize);
	fprintf(stderr, "infohead.width = %d\n", (int)infohead.biWidth);
	fprintf(stderr, "infohead.height = %d\n", (int)infohead.biHeight);
	fprintf(stderr, "infohead.biPlanes = %d\n", infohead.biPlanes);
	fprintf(stderr, "infohead.biBitCount = %d\n", infohead.biBitCount);
#endif

	gsize = freader.bfSize - freader.bfOffBits;
	lsize = gsize/infohead.biHeight;
//	printf("lsize = %d\n", lsize);
	gdata = (unsigned char *)malloc(gsize);

	printf("# %s %d %d\n", name , (int)infohead.biWidth,
	       (int)infohead.biHeight);
	
	return 0;
}

int load_bmpdata(FILE *fp)
{
	int j;
	unsigned char *dp = gdata;
	size_t sz;
	
	for(j=0; j<infohead.biHeight; j++) {
		fseek(fp, freader.bfOffBits
		      +(infohead.biHeight-j-1)*lsize,
		      SEEK_SET);
		sz = fread(dp, lsize, 1, fp);
		if(sz != 1) {
			fprintf(stderr, "Cannot read data.(%zd)\n", sz);
			return -1;
		}
		dp += lsize;
	}
	
	return 0;
}

void disp_bmp(void)
{
	int i, j;
	unsigned char *dp = gdata;
	
	for(j=0; j<infohead.biHeight; j++) {
		for(i=0; i<((infohead.biWidth+7)/8); i++) {
			unsigned char bit = 0x80;
			int k;
			for(k=0; k<8; k++) {
				if(*dp & bit) {
					printf(". ");
				} else {
					printf("X ");
				}
				bit >>= 1;
			}
			dp ++;
		}
		printf("\n");
	}
}

void bmp2src(void)
{
	int i, j;
	unsigned char *dp = gdata;
	
	for(j=0; j<infohead.biHeight; j+=8) {
		for(i=0; i<((infohead.biWidth+7)/8); i++) {
			unsigned char bit = 0x80;
			int k;
			for(k=0; k<8; k++) {
				if(*dp & bit) {
					printf(" ");
				} else {
					printf("#");
				}
				bit >>= 1;
			}
			dp ++;
		}
		printf("\n");
	}
}

#define MAXNAMELEN	32

int main(int argc, char *argv[])
{
	FILE *fp = 0;
	char bname[MAXNAMELEN+1], *p;;
	int i;
	
	if(argc > 1) {
		fp = fopen(argv[1], "r");
		if(fp == 0) {
			fprintf(stderr, "cannot open \"%s\".\n", argv[1]);
			return -1;
		}
	} else {
		fprintf(stderr, "Usage: %s <bmp_file>\n", argv[0]);
		return 0;
	}

	p = argv[1];
	for(i=0; i<MAXNAMELEN; i++) {
		if((*p != '.') && (*p != 0)) {
			bname[i] = *p;
			p++;
		} else {
			bname[i] = 0;
			break;
		}
	}
	
	if(load_bmpheader(fp, bname)) return -1;
	if(load_bmpdata(fp)) return -1;
	disp_bmp();

	return 0;
}

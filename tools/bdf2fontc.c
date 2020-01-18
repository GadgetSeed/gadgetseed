#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSTR	256
#define MAXBUF	64
#define MAXINDEX	0x10000

extern const unsigned short jis2uni_table[];

int flg_jisx = 0;
char fontname[256];
char *sectname = 0;
int fwidth, fheight;
int origin_x, origin_y;
int max_width = 0, max_height = 0;
int s_ch = 0xffff, e_ch = 0;
int font_count = 0;
int byte_count = 0;
int font_start_byte = 0;
int fascent = 0;
int fdescent = 0;

int c_num = 0;
unsigned int findex[MAXINDEX];

void tohpat(int ch, int buf[MAXBUF], int dwidth, int width, int height, int osx, int osy)
{
	int i, j;

	printf("// CODE 0x%04x\n", ch);
	printf("// NUM %d", c_num);
	if((ch >= 0x20) && (ch < 0x7f)) {
		printf(" \"%c\"", ch);
	}
	printf("\n");
	printf("// %d %d %d %d %d\n", dwidth, width, height, osx, osy);

	for(j=0; j<height; j++) {
		printf("// ");
		for(i=0; i<width; i++) {
			if(buf[j] & ((int)1<<(((width+7)/8)*8-i-1))) {
				printf("X ");
			} else {
				printf(". ");
			}
		}
		printf("\n");
	}

//	printf("	{\n");
	printf("// BYTE %d %08X\n", byte_count, byte_count);
	printf("	%d,	// dwidth\n", dwidth);
	printf("	%d,	// width\n", width);
	printf("	%d,	// height\n", height);
	printf("	%d,	// offset X\n", osx/* - origin_x*/); // [TODO]
	printf("	%d,	// offset Y\n", osy + fdescent/* - origin_y*/); // [TODO]
	byte_count += 5;

	for(j=0; j<height; j++) {
		printf("		");
		for(i=0; i<((width+7)/8); i++) {
			printf("0x%02x, ", (int)((buf[j]>>(((width+7)/8)-i-1)*8)) & 0xff);
			byte_count ++;
		}
		printf("\n");
	}
//	printf("	},\n");

	font_count ++;

	if((ch != 0x2e3a)
	   && (ch != 0x2e3b)
	   && (ch != 0x3031)
	   && (ch != 0x3032)
	   && (ch != 0x1eb6)
	   ) {
		if(max_width < width) {
			max_width = width;
		}
		if(max_height < height) {
			max_height = height;
		}
	}
}

static struct {
	int code_start;
	int code_end;
} code_table[0x100];

void make_index(void)
{
	int i;
	int seg_dtcnt = 0, dtotal = 0;
	int start_code = 0xffff, end_code = 0;
	int table_num = 0;

	printf("// CODE 0x%04x - 0x%04x (TABLE %d bytes %d KB)\n", s_ch, e_ch,
	       (e_ch - s_ch + 1) * 4,
	       (e_ch - s_ch + 1) * 4 / 1024);
	printf("// %d charcters (BITDATA %d bytes %d KB)\n", c_num,
	       byte_count, byte_count/1024);

	for(i=s_ch; i<=e_ch; i++) {
		if((i != s_ch) && (findex[i] == 0xffffffff)) {
			// フォント無し
		} else {
			if(i < start_code) start_code = i;
			if(end_code < i) end_code = i;
			seg_dtcnt ++;
		}
		if(((i & 0xff) == 0xff) || (i == e_ch)) {
			if(seg_dtcnt != 0) {
				fprintf(stderr, "%04X - %04X : %d\n", start_code, end_code, seg_dtcnt);
			} else {
				fprintf(stderr, "%04X - %04X : %d\n", i - 0xfff, i, seg_dtcnt);
			}
			dtotal += seg_dtcnt;
			code_table[i>>8].code_start = start_code;
			code_table[i>>8].code_end = end_code;
			seg_dtcnt = 0;
			start_code = 0xffff;
			end_code = 0;
		}
	}
	fprintf(stderr, "TOTAL : %d\n", dtotal);

	for(i=0; i<=0xff; i++) {
		int cnum = 0;
		if(code_table[i].code_start == 0xffff) {
			cnum = 0;
		} else {
			int j;
			cnum = code_table[i].code_end - code_table[i].code_start + 1;
			fprintf(stderr, "%X : %04X - %04X (%d)\n", i,
				code_table[i].code_start,
				code_table[i].code_end,
				cnum);

			printf("// %X : %04X - %04X (%d)\n", i,
			       code_table[i].code_start,
			       code_table[i].code_end,
			       cnum);
			printf("static const unsigned int font_index_%s_%02X[] = {\n", fontname, i);
			printf("	0x%08x,	// START\n", code_table[i].code_start & 0xff);
			printf("	0x%08x,	// END\n", code_table[i].code_end & 0xff);
			for(j=code_table[i].code_start; j<=code_table[i].code_end; j++) {
				printf("	0x%08x,	// 0x%04x\n", findex[j], j);
			}
			printf("};\n");

			table_num += cnum;
		}
	}
	fprintf(stderr, "TABLE : %d\n", table_num);

	printf("static const unsigned int *code_index_%s[0x100] = {\n", fontname);
	for(i=0; i<=0xff; i++) {
		if(code_table[i].code_start == 0xffff) {
			printf("	0,	// %04X\n", i*0x100);
		} else {
			printf("	font_index_%s_%02X,	// %04X\n", fontname, i, i*0x100);
		}
	}
	printf("};\n");

	printf("#include \"fontdata.h\"\n");
	printf("const struct st_font font_%s = {\n", fontname);
	printf("	.start	= 0x%04x,\n", s_ch);
	printf("	.end	= 0x%04x,\n", e_ch);
	printf("	.width	= %d,\n", fwidth);
	//printf("	.height	= %d,\n", fheight);
	printf("	.height	= %d,\n", fascent + fdescent);
	printf("	.dwidth	= %d,\n", ((fwidth+7)/8)*fheight+2);// start:1byte width:1byte
	printf("	.index	= (unsigned int *)code_index_%s,\n", fontname);
	printf("	.bitmap	= (signed char *)font_bitmap_%s,\n", fontname);
	printf("};\n");
}

int main(int argc, char *argv[])
{
	FILE *fp = 0;
	int width, height;
	int dwidth, dheight;
	int osx, osy;
	int ch = 0;
	int lch = 0, sch = -1;
	char str[MAXSTR];
	char charsetreg[MAXSTR];
	int i;

	if(argc > 1) {
		fp = fopen(argv[1], "r");
		if(fp == 0) {
			printf("cannot open \"%s\".\n", argv[1]);
			return -1;
		}

		if(argc > 2) {
			strncpy(fontname, argv[2], 256);
		}

		if(argc > 3) {
			sectname = argv[3];
		}
	} else {
		printf("Usage: %s <bdf> [fontname] [section]\n", argv[0]);
		return 0;
	}

	for(i=0; i<MAXINDEX; i++) {
		findex[i] = 0xffffffff;
	}

	while(fgets(str, MAXSTR, fp) != NULL) {
		if(strncmp(str, "CHARSET_REGISTRY ", 17) == 0) {
			sscanf(str, "CHARSET_REGISTRY %s", charsetreg);
			fprintf(stderr, "CHARSET_REGISTRY = %s\n", charsetreg);
			if((strncmp("\"JISX02", charsetreg, 7) == 0) ||
			   (strncmp("\"jisx02", charsetreg, 7) == 0)) {
				flg_jisx = 1;
				fprintf(stderr, "JISX charset\n");
			}
		}

		if(strncmp(str, "FONTBOUNDINGBOX ", 16) == 0) {
			sscanf(str, "FONTBOUNDINGBOX %d %d %d %d", &fwidth, &fheight, &origin_x, &origin_y);
		}

		if(strncmp(str, "PIXEL_SIZE ", 11) == 0) {
			sscanf(str, "PIXEL_SIZE %d", &fheight);
		}

		if(strncmp(str, "FONT_ASCENT ", 12) == 0) {
			sscanf(str, "FONT_ASCENT %d", &fascent);
		}

		if(strncmp(str, "FONT_DESCENT ", 13) == 0) {
			sscanf(str, "FONT_DESCENT %d", &fdescent);
		}

		if(strncmp(str, "FOUNDRY ", 8) == 0) {
			if(fontname[0] == 0) {
				sscanf(str, "FOUNDRY \"%s", fontname);
				fontname[strlen(fontname)-1] = 0;
			}
			if(sectname == 0) {
				printf("const signed char "
				       "font_bitmap_%s[]"
				       " = {\n", fontname);
			} else {
				printf("const signed char "
				       "font_bitmap_%s[]"
				       " __attribute__ ((section(\"%s\")))"
				       " = {\n", fontname, sectname);
			}
		}

		if(strncmp(str, "ENCODING ", 9) == 0) {
			sscanf(str, "ENCODING %d", &ch);

			if(flg_jisx != 0) {
				ch = jis2uni_table[ch];
			}

			if(ch != 0) {
				if(sch == -1) sch = ch;

				if(ch < s_ch) s_ch = ch;
				if(e_ch < ch) e_ch = ch;

				if((ch != 0) && (lch != 0) && ((ch - lch) > 1)) {
					//printf("// 0x%04x - 0x%04x %d\n", sch, lch, lch-sch+1);
					sch = ch;
				}
				lch = ch;
			}
		}

		if(strncmp(str, "DWIDTH ", 4) == 0) {
			sscanf(str, "DWIDTH %d %d", &dwidth, &dheight);
		}

		if(strncmp(str, "BBX ", 4) == 0) {
			sscanf(str, "BBX %d %d %d %d", &width, &height, &osx, &osy);
		}

		if(strncmp(str, "BITMAP", 6)==0) {
			int l = 0;
			int buf[MAXBUF];

			while(fgets(str, MAXSTR, fp) != NULL) {
				if(strncmp(str, "ENDCHAR", 7) == 0) {
					break;
				}

				sscanf(str, "%x", &buf[l]);
				l ++;
			}

			if(ch != 0) {
				tohpat(ch, buf, dwidth, width, height, osx, osy);

				//findex[ch] = c_num;
				if(findex[ch] == 0xffffffff) {
					findex[ch] = font_start_byte;
				}
				font_start_byte = byte_count;
				c_num ++;
			}
		}
	}

	printf("};\n");

	make_index();

	fprintf(stderr, "Font count   : %d\n", font_count);
	fprintf(stderr, "Byte count   : %d\n", byte_count);
	fprintf(stderr, "Font ascent  : %d\n", fascent);
	fprintf(stderr, "Font descent : %d\n", fdescent);
	fprintf(stderr, "Font width   : %d\n", fwidth);
	fprintf(stderr, "Font height  : %d\n", fheight);
	fprintf(stderr, "Origin X     : %d\n", origin_x);
	fprintf(stderr, "Origin Y     : %d\n", origin_y);
	fprintf(stderr, "MAX width    : %d\n", max_width);
	fprintf(stderr, "MAX height   : %d\n", max_height);

	return 0;
}

#include <stdio.h>
#include <string.h>

#define MAXSTR	256
#define MAXBUF	256
#define MAXINDEX	0x10000

char fontname[256];
int fwidth, fheight;
int s_ch = 0xffff, e_ch = 0;
int font_count = 0;
int byte_count = 0;
int font_start_byte = 0;

int c_num = 0;
unsigned int findex[MAXINDEX];

void tohpat(int ch, unsigned long long buf[MAXBUF], int start, int width)
{
	int i, j;

	printf("// 0x%04x 0x%04x", ch, c_num);
	if((ch >= 0x20) && (ch < 0x7f)) {
		printf(" \"%c\"", ch);
	}
	printf("\n");
	printf("// %d %d\n", start, width);

	for(j=0; j<fheight; j++) {
		printf("// ");
		for(i=0; i<fwidth; i++) {
			if(buf[j] & ((unsigned long long)1<<(((fwidth+7)/8)*8-i-1))) {
				printf("X ");
			} else {
				printf(". ");
			}
		}
		printf("\n");
	}

	printf("	%d,	// dwidth\n", fwidth);
	printf("	%d,	// width\n", width);
	printf("	%d,	// height\n", fheight);
	printf("	%d,	// offset X\n", start);
	printf("	%d,	// offset Y\n", 0);
	byte_count += 5;

	for(j=0; j<fheight; j++) {
		printf("		");
		for(i=0; i<((fwidth+7)/8); i++) {
			printf("0x%02x, ", (int)((buf[j]>>(((fwidth+7)/8)-i-1)*8)) & 0xff);
			byte_count ++;
		}
		printf("\n");
	}
}

void make_index(void)
{
	int i;

	printf("// 0x%04x - 0x%04x (%d bytes %d KB)\n", s_ch, e_ch,
	       (e_ch - s_ch + 1) * fwidth * 2,
	       (e_ch - s_ch + 1) * fwidth * 2 / 1024);
	printf("// %d charcters (%d bytes %d KB)\n", c_num,
	       c_num * fwidth * 2,
	       c_num * fwidth * 2 / 1024);

	printf("const unsigned int font_index_%s[] = {\n",
	       fontname);
	printf("	0x%08x,	// START\n", s_ch);
	printf("	0x%08x,	// END\n", e_ch);

	for(i=s_ch; i<=e_ch; i++) {
		if((i != s_ch) && (findex[i] == 0)) {
			findex[i] = 0xffffffff;
		}
		printf("	0x%08x,	// 0x%08x\n", findex[i], i);
	}

	printf("};\n");

	printf("static const unsigned int *code_index_%s[1] = {\n", fontname);
	printf("	font_index_%s\n", fontname);
	printf("};\n");

	printf("#include \"fontdata.h\"\n");
	printf("const struct st_font font_%s = {\n", fontname);
	printf("	.start	= 0x%04x,\n", s_ch);
	printf("	.end	= 0x%04x,\n", e_ch);
	printf("	.width	= %d,\n", fwidth);
	printf("	.height	= %d,\n", fheight);
	printf("	.dwidth	= %d,\n", ((fwidth+7)/8)*fheight+2);// start:1byte width:1byte
	printf("	.index	= (unsigned int *)code_index_%s,\n", fontname);
	printf("	.bitmap	= (signed char *)font_bitmap_%s,\n", fontname);
	printf("};\n");
}

int main(int argc, char *argv[])
{
	FILE *fp = 0;
	int ch = 0, lch = 0, sch = -1;
	int start, width;
	char str[MAXSTR];

	if(argc > 1) {
		fp = fopen(argv[1], "r");
		if(fp == 0) {
			printf("cannot open \"%s\".\n", argv[1]);
			return -1;
		}

		if(argc > 2) {
			strncpy(fontname, argv[2], 256);
		}
	} else {
		printf("Usage: %s <fontdata>\n", argv[0]);
		return 0;
	}

	while(fgets(str, MAXSTR, fp) != NULL) {
		if(strncmp(str, "SIZE ", 5) == 0) {
			sscanf(str, "SIZE %d %d", &fwidth, &fheight);
			printf("const unsigned char "
			       "font_bitmap_%s[] = {\n", fontname);
		}

		if(fontname[0] == 0) {
			if(strncmp(str, "NAME ", 5) == 0) {
				sscanf(str, "NAME %s", fontname);
				fontname[strlen(fontname)] = 0;
			}
		}

		if(strncmp(str, "# ", 2) == 0) {
			sscanf(str, "# %x %d %d", &ch, &start, &width);

			if(sch == -1) sch = ch;

			if(ch < s_ch) s_ch = ch;
			if(e_ch < ch) e_ch = ch;

			if((ch != 0) && (lch != 0) && ((ch - lch) > 1)) {
				printf("// 0x%04x - 0x%04x %d\n",
				       sch, lch, lch-sch+1);
				sch = ch;
			}
			lch = ch;

			{
				int i, j;
				char tmp[MAXSTR];
				unsigned long long buf[MAXBUF];
				char *rt;

				for(j=0; j<fheight; j++) {
					buf[j] = 0;

					rt = fgets(tmp, MAXSTR, fp);

					if(rt != tmp) {
						printf("error data.\n");
						return -1;
					}

					for(i=0; i<fwidth; i++) {
						buf[j] <<= 1;
						if(tmp[i*2] != '.') {
							buf[j] |= 1;
						}
					}
					buf[j] <<= ((8-(fwidth & 7)) % 8);
				}

				tohpat(ch, buf, start, width);

				//findex[ch] = c_num;
				findex[ch] = font_start_byte;
				font_start_byte = byte_count;
				c_num ++;
			}
		}

	}

	printf("};\n");

	make_index();

	return 0;
}

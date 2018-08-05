#include <stdio.h>
#include <string.h>

#define MAXSTR	1024

#define MAXWIDTH	256
#define MAXHEIGHT	256

unsigned char graph[MAXHEIGHT][MAXWIDTH];

void tohpat(char *name, int width, int height)
{
	int i, j, k;

	printf("const unsigned char %s_data[] = {\n", name);

	for(j=0; j<height; j++) {
		printf("// ");
		for(i=0; i<width; i++) {
			if(graph[j][i] != 0) {
				printf("X ");
			} else {
				printf(". ");
			}
		}
		printf("\n");
	}

	for(j=0; j<height; j++) {
		for(i=0; i<width; i+=8) {
			int bit = 0x80;
			int pat = 0;

			for(k=0; k<8; k++) {
				if(graph[j][i+k]) {
					pat |= bit ;
				}
				bit >>= 1;
			}

			printf("	");
			printf("0x%02x,\n", pat);
		}
		printf("\n");
	}

	printf("};\n");
	printf("\n");
	printf("const struct st_bitmap %s = {\n", name);
	printf("	%d,	// width\n", width);
	printf("	%d,	// height\n", height);
	printf("	(unsigned char *)%s_data	// data\n", name);
	printf("};\n");
}

int main(int argc, char *argv[])
{
	FILE *fp = 0;
	int width, height;
	char name[MAXSTR];
	char str[MAXSTR];

	if(argc > 1) {
		fp = fopen(argv[1], "r");
		if(fp == 0) {
			printf("cannot open \"%s\".\n", argv[1]);
			return -1;
		}
	}

	printf("#include \"graphics.h\"\n\n");

	while(fgets(str, MAXSTR, fp) != NULL) {
		if(strncmp(str, "# ", 2) == 0) {
			int i, j;
			char tmp[MAXSTR];

			for(j=0; j<MAXHEIGHT; j++) {
				for(i=0; i<MAXWIDTH; i++) {
					graph[j][i] = 0;
				}
			}

			sscanf(str, "# %s %d %d", name, &width, &height);

			for(j=0; j<height; j++) {
				if(fgets(tmp, MAXSTR, fp) != tmp) {
					printf("data error.\n");
					return -1;
				}
				for(i=0; i<width; i++) {
					if(tmp[i*2] != '.') {
						graph[j][i] = 1;
					} else {
						graph[j][i] = 0;
					}
				}
			}

			tohpat(name, width, height);
		}

	}

	return 0;
}

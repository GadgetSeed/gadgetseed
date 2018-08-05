CODETABLE = iso2022_unicode_tbl.c

TOOLS = bdf2fontc txt2fontc bmp2txt txt2bitmap

all: $(TOOLS)

iso-2022-jp-2004-std.txt:
	wget http://x0213.org/codetable/iso-2022-jp-2004-std.txt

$(CODETABLE): iso-2022-jp-2004-std.txt mkiso2022touni.awk
	gawk -f mkiso2022touni.awk $< > $@

bdf2fontc: bdf2fontc.c $(CODETABLE)
	gcc -Wall -O2 -o $@ $< $(CODETABLE)

txt2fontc: txt2fontc.c
	gcc -Wall -O2 -o $@ $<

bmp2txt: bmp2txt.c
	gcc -Wall -O2 -o $@ $<

txt2bitmap: txt2bitmap.c
	gcc -Wall -O2 -o $@ $<

clean:
	rm -f $(CODETABLE) $(TOOLS) *~

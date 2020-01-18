FONTNAME = jiskan16gs

FONTTXT  = ../8x16/font_8x16.txt

DBFONTARC  = ../jiskan16/jiskan16-1990.bdf.Z
DBFONTBDF  = ../jiskan16/jiskan16-1990.bdf
DBFONTNAME = Misc

$(DBFONTBDF): $(DBFONTARC)
	uncompress -c $(DBFONTARC) > $@

$(DBFONTARC):
	wget http://pkgs.fedoraproject.org/repo/pkgs/japanese-bitmap-fonts/jiskan16-1990.bdf.Z/md5/77d997789026b1829bb4a33f65a0ce78/$(DBFONTARC)
#	wget http://kanji.zinbun.kyoto-u.ac.jp/~yasuoka/ftp/fonts/$(DBFONTARC)

include $(PRJ_DIR)/$(FONTDATA_DIR)/txtbdf2font.mk

distclean:
	rm -f $(DBFONTARC) $(DBFONTBDF)

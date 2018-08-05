FONTNAME = jiskan16

OBFONTBDF   = 8x16rk.bdf
OBFONTNAME  = Sony

DBFONTARC  = jiskan16-1990.bdf.Z
DBFONTBDF  = jiskan16-1990.bdf
DBFONTNAME = Misc

$(OBFONTBDF):
	wget http://ftp.iij.ad.jp/pub/FreeBSD-jp/ports-jp/LOCAL_PORTS/8x16rk.bdf
#	wget ftp://ftp.iij.ad.jp/pub/NetBSD/NetBSD-release-5/xsrc/external/mit/font-sony-misc/dist/8x16rk.bdf

$(DBFONTBDF): $(DBFONTARC)
	uncompress -c $(DBFONTARC) > $@

$(DBFONTARC):
	wget http://pkgs.fedoraproject.org/repo/pkgs/japanese-bitmap-fonts/jiskan16-1990.bdf.Z/md5/77d997789026b1829bb4a33f65a0ce78/$(DBFONTARC)
#	wget http://kanji.zinbun.kyoto-u.ac.jp/~yasuoka/ftp/fonts/$(DBFONTARC)

include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2dbfont.mk

distclean:
	rm -f $(DBFONTARC) $(OBFONTBDF) $(DBFONTBDF)

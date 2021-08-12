FONTNAME = knj10

FONTARC = knj10-1.1.tar.gz
FONTDIR = knj10-1.1

OBFONTBDF  = $(FONTDIR)/5x10rk.bdf
OBFONTNAME = knj10_ob

DBFONTBDF  = $(FONTDIR)/knj10.bdf
DBFONTNAME = knj10_db

$(OBFONTBDF): $(FONTARC)
	tar xvfz $(FONTARC)
	touch $(OBFONTBDF) $(DBFONTBDF)

$(FONTARC):
	wget http://ftp.lip6.fr/pub/minix/distfiles/backup/$(FONTARC)
#	wget http://hp.vector.co.jp/authors/VA013391/fonts/$(FONTARC)

include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2dbfont.mk

distclean:
	rm -rf $(FONTARC) $(FONTDIR)

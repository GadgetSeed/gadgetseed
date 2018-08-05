FONTNAME = misaki

FONTARC = mskbdb10.tar.gz
FONTDIR = .

OBFONTBDF  = $(FONTDIR)/4x8.bdf
OBFONTNAME = misaki_ob

DBFONTBDF  = $(FONTDIR)/misaki.bdf
DBFONTNAME = misaki_db

$(OBFONTBDF): $(FONTARC)
	tar xvfz $(FONTARC)

$(FONTARC):
	wget http://www.geocities.jp/littlimi/arc/misaki/$(FONTARC)

include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2dbfont.mk

distclean:
	rm -rf $(FONTARC) *.bdf

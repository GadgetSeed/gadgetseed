FONTNAME = misaki

FONTARC = misaki_bdf_2019-06-03.zip
#FONTARC = mskbdb10.tar.gz
FONTDIR = .

#OBFONTBDF  = $(FONTDIR)/4x8.bdf
#OBFONTNAME = misaki_ob

BDFFONT  = $(FONTDIR)/misaki_gothic.bdf
FONTNAME = misaki
#DBFONTBDF  = $(FONTDIR)/misaki.bdf
#DBFONTNAME = misaki_db

#$(OBFONTBDF): $(FONTARC)
#	tar xvfz $(FONTARC)
$(BDFFONT): $(FONTARC)
	unzip -o $(FONTARC)

$(FONTARC):
	wget http://littlelimit.net/arc/misaki/$(FONTARC)
#	wget http://www.geocities.jp/littlimi/arc/misaki/$(FONTARC)

#include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2dbfont.mk
include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2fontc.mk

distclean:
	rm -rf $(FONTARC) *.bdf *.[co] *.txt

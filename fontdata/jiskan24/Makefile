FONTNAME = jiskan24

FONTTXT  = ../12x24/font_12x24.txt

DBFONTARC  = jiskan24-2003-1.bdf.bz2
DBFONTBDF  = jiskan24-2003-1.bdf
DBFONTNAME = kan24x24

$(DBFONTBDF): $(DBFONTARC)
	bunzip2 -f $(DBFONTARC)
	touch $(DBFONTBDF)

$(DBFONTARC):
	wget http://poiut.web.fc2.com/computer/$(DBFONTARC)
#	touch $(DBFONTARC)

include $(PRJ_DIR)/$(FONTDATA_DIR)/txtbdf2font.mk

distclean:
	rm -f $(DBFONTARC) $(DBFONTBDF)

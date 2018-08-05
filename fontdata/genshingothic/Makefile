FONTNAME = genshingothic_p18
FONTFILE = GenShinGothic-Monospace-Normal

TTFFONTARC	= genshingothic-20150607.7z
TTFFONT		= $(FONTFILE).ttf
BDFFONT		= $(FONTNAME).bdf

$(TTFFONTARC):
	wget https://osdn.jp/downloads/users/8/8634/$(TTFFONTARC)

$(TTFFONT): $(TTFFONTARC)
	7z x -aoa $(TTFFONTARC)
	touch $(TTFFONT)

$(BDFFONT): $(TTFFONT)
	-otf2bdf -p 18 $(TTFFONT) > $(BDFFONT)

include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2fontc.mk

clean:
	rm -f $(BDFFONT)

distclean:
	rm -f $(TTFFONTARC) $(BDFFONT) $(FONTSRC)

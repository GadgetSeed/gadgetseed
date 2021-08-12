FONTNAME = mplus_p2_p18
FONTFILE = mplus-2p-regular

TTFFONTARC	= mplus-TESTFLIGHT-063a.tar.xz
TTFFONT		= mplus-TESTFLIGHT-063a/$(FONTFILE).ttf
BDFFONT		= $(FONTNAME).bdf

$(TTFFONTARC):
	wget https://ja.osdn.net/dl/mplus-fonts/$(TTFFONTARC)

$(TTFFONT): $(TTFFONTARC)
	tar xvf $(TTFFONTARC)
	touch $(TTFFONT)

$(BDFFONT): $(TTFFONT)
	-otf2bdf -p 20 $(TTFFONT) > $(BDFFONT)

include $(PRJ_DIR)/$(FONTDATA_DIR)/bdf2fontc.mk

distclean:
	rm -f $(TTFFONTARC) $(BDFFONT) $(FONTSRC)

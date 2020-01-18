include $(PRJ_DIR)/common.mk

TOOLSDIR= $(PRJ_DIR)/tools
FONTCNV = $(TOOLSDIR)/bdf2fontc
FONTCNV2= $(TOOLSDIR)/txt2fontc

SRCS	 = $(FONTNAME).c

OBJS     = $(SRCS:%.c=%.o)

all: $(OBJS)

$(OBJS): $(SRCS)

$(SRCS): $(FONTTXT) $(FONTCNV) $(FONTCNV2) $(DBFONTBDF)
	$(FONTCNV2) $(FONTTXT) $(FONTNAME) > $@
	$(FONTCNV) $(DBFONTBDF) $(DBFONTNAME) $(FONTS_MAP_BITMAPDATA_EXTROM) >> $@
	echo "#include \"font.h\"" >> $@
	echo "#include \"sysconfig.h\"" >> $@
	echo "const struct st_fontset fontset_$(FONTNAME) = {" >> $@
	echo "	.name	= \"$(FONTNAME)\"," >> $@
	echo "	.font	= (struct st_font *)&font_$(FONTNAME)," >> $@
	echo "	.w_font	= (struct st_font *)&font_$(DBFONTNAME)," >> $@
	echo "};" >> $@
	echo "const struct st_fontset * const fontptr_$(FONTNAME) = &fontset_$(FONTNAME);" >> $@

clean:
	rm -f -r $(OBJS) $(SRCS)

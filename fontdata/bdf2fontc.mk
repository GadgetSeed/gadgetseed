include $(PRJ_DIR)/common.mk

CFLAGS	+= -I$(PRJ_DIR)/include

TOOLSDIR= $(PRJ_DIR)/tools
FONTCNV = $(TOOLSDIR)/bdf2fontc

SRCS	 = $(FONTNAME).c

OBJS     = $(SRCS:%.c=%.o)

all: $(OBJS)

$(OBJS): $(SRCS)

$(SRCS): $(BDFFONT) $(FONTCNV)
	$(FONTCNV) $(BDFFONT) $(FONTNAME) $(FONTS_MAP_BITMAPDATA_EXTROM) > $@
	echo "#include \"font.h\"" >> $@
	echo "#include \"sysconfig.h\"" >> $@
	echo "const struct st_fontset fontset_$(FONTNAME) = {" >> $@
	echo "	.name	= \"$(FONTNAME)\"," >> $@
	echo "	.font	= (struct st_font *)&font_$(FONTNAME)," >> $@
	echo "};" >> $@
	echo "const struct st_fontset * const fontptr_$(FONTNAME) = &fontset_$(FONTNAME);" >> $@

clean:
	rm -f -r $(OBJS) $(SRCS)

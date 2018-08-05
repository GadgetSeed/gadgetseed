include $(PRJ_DIR)/common.mk

TOOLSDIR= $(PRJ_DIR)/tools

LIB	= fontdata.a

OBJS = 

ifeq ($(FONTS_ENABLE_FONT_4X6),YES)
	OBJS += 4x6/4x6.o
endif

ifeq ($(FONTS_ENABLE_FONT_4X8),YES)
	OBJS += 4x8/4x8.o
endif

ifeq ($(FONTS_ENABLE_FONT_6X6),YES)
	OBJS += 6x6/6x6.o
endif

# default font
#ifeq ($(FONTS_ENABLE_FONT_8X16),YES)
	OBJS += 8x16/8x16.o
#endif

ifeq ($(FONTS_ENABLE_FONT_12X16),YES)
	OBJS += 12x16/12x16.o
endif

ifeq ($(FONTS_ENABLE_FONT_12X24),YES)
	OBJS += 12x24/12x24.o
endif

ifeq ($(FONTS_ENABLE_FONT_16X24),YES)
	OBJS += 16x24/16x24.o
endif

ifeq ($(FONTS_ENABLE_FONT_NUM24X32),YES)
	OBJS += num24x32/num24x32.o
endif

ifeq ($(FONTS_ENABLE_FONT_NUM24X40),YES)
	OBJS += num24x40/num24x40.o
endif

ifeq ($(FONTS_ENABLE_FONT_NUM24X48),YES)
	OBJS += num24x48/num24x48.o
endif

ifeq ($(FONTS_ENABLE_FONT_NUM48X64),YES)
	OBJS += num48x64/num48x64.o
endif

ifeq ($(FONTS_ENABLE_FONT_MISAKI),YES)
	OBJS += misaki/misaki.o
endif

ifeq ($(FONTS_ENABLE_FONT_NAGA10),YES)
	OBJS += knj10/knj10.o
endif

ifeq ($(FONTS_ENABLE_FONT_JISKAN16),YES)
	OBJS += jiskan16/jiskan16.o
endif

ifeq ($(FONTS_ENABLE_FONT_JISKAN24),YES)
	OBJS += jiskan24/jiskan24.o
endif

ifeq ($(FONTS_ENABLE_FONT_GENSHINGOTHIC),YES)
	OBJS += genshingothic/genshingothic_p18.o
endif

ifeq ($(FONTS_ENABLE_FONT_MPLUS),YES)
	OBJS += m+/mplus_p2_p18.o
endif

all: $(LIB)

$(LIB): $(OBJS) $(MAKECONFIGSYS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

4x8/4x8.o:
	make -C 4x8 all

4x6/4x6.o:
	make -C 4x6 all

6x6/6x6.o:
	make -C 6x6 all

8x16/8x16.o: 8x16/font_8x16.txt
	make -C 8x16 all

12x16/12x16.o: 12x16/font_12x16.txt
	make -C 12x16 all

12x24/12x24.o: 12x24/font_12x24.txt
	make -C 12x24 all

16x24/16x24.o: 16x24/font_16x24.txt
	make -C 16x24 all

num24x32/num24x32.o: num24x32/font_num24x32.txt
	make -C num24x32 all

num24x40/num24x40.o: num24x40/font_num24x40.txt
	make -C num24x40 all

num24x48/num24x48.o: num24x48/font_num24x48.txt
	make -C num24x48 all

num48x64/num48x64.o: num48x64/font_num48x64.txt
	make -C num48x64 all

misaki/misaki.o:
	make -C misaki all

knj10/knj10.o:
	make -C knj10 all

jiskan16/jiskan16.o:
	make -C jiskan16 all

jiskan24/jiskan24.o: 12x24/font_12x24.txt
	make -C jiskan24 all

genshingothic/genshingothic_p18.o:
	make -C genshingothic all

m+/mplus_p2_p18.o:
	make -C m+ all

clean:
	rm -f -r $(OBJS) $(SRCS) $(LIB) $(DEPEND) *~
	make -C 4x8 clean
	make -C 4x6 clean
	make -C 6x6 clean
	make -C num24x32 clean
	make -C num24x40 clean
	make -C num24x48 clean
	make -C num48x64 clean
	make -C 8x16 clean
	make -C 12x16 clean
	make -C 12x24 clean
	make -C 16x24 clean
	make -C misaki clean
	make -C knj10 clean
	make -C jiskan16 clean
	make -C jiskan24 clean
	make -C genshingothic clean
	make -C m+ clean

distclean:
	make clean
	make -C jiskan16 distclean
	make -C jiskan24 distclean
	make -C knj10 distclean
	make -C misaki distclean
	make -C genshingothic distclean
	make -C m+ distclean

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)

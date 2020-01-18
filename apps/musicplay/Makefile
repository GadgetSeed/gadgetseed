include $(PRJ_DIR)/common.mk

TXT2BITMAP = $(PRJ_DIR)/tools/txt2bitmap

LIB	= musicplay.a

OBJS	= musicplay.o musicplay_view.o filesearch.o filelist.o \
	  sdmusic.o sdmusic_ctrl_view.o config_view.o clock_view.o \
	  musicinfo_view.o playtime_slider.o list_view.o volume_view.o spectrum_view.o \
	  settings_view.o

CFLAGS	+= -I$(PRJ_DIR)/$(UILIB_DIR) -I$(PRJ_DIR)/apps/soundplay

ifdef ENABLE_MUSICPLAY_INTERNETRADIO
OBJS	+= mode_view.o radio.o radio_ctrl_view.o radiolist_view.o radiobuffer_view.o
CFLAGS  += -I$(PRJ_DIR)/apps/internetradio
endif

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)

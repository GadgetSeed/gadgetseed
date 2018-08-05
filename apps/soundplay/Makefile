include $(PRJ_DIR)/common.mk

LIB	= soundplay.a

OBJS	= soundplay.o \
	  wavplay.o \
	  mp3play.o \
	  m4aplay.o \
	  music_info.o \
	  id3tag.o \
	  mp4tag.o \
	  spectrum_analyzer.o \
	  fft.o

CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/libmad/libmad-0.15.1b
CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/faad2/faad2-2.7/include

ifeq ($(LIB_ENABLE_PICOJPEG),YES)
CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/picojpeg
endif

ifeq ($(LIB_ENABLE_LIBPNG),YES)
CFLAGS += -I$(PRJ_DIR)/$(EXTLIBS_DIR)/libpng
endif

all:	$(LIB)

$(LIB): $(OBJS)
	$(AR) rc $(LIB) $(OBJS)
	$(RANLIB) $(LIB)

clean:
	rm -f -r $(OBJS) $(LIB) $(DEPEND)

include $(PRJ_DIR)/rules.mk

-include $(DEPEND)
